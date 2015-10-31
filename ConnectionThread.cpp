#include <iostream>

#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/network/TcpSocket.h"
#include "./m2gnetwork/network/Socket.h"
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/rdr/types.h"

#include "./m2gnetwork/network/Network_API.h"
#include "ConnectionThread.h"
#include "h264Stream.h"
#include "config.h"

using namespace network;
using namespace rdr;
using namespace std;

pthread_mutex_t mutex_t;

ConnectionThread::ConnectionThread()
{
	cout << "ConnectionThread::ConnectionThread() is called" << endl;
	ptr = this;
	First = true;
	Send = false;
	WaitIFrame = false;
	//
	len=0;
	Send = false;
	WaitFrame = true; 
	//
}

void ConnectionThread::setArbiter( int num )
{
	Arbiter = num;
}

bool ConnectionThread::IsFirst()
{
	return First;
}

void ConnectionThread::SetSend( bool OK )
{
	Send = OK;
}

void ConnectionThread::ChangeFirst( void )
{
	First = false;
	WaitIFrame = true;
}

void ConnectionThread::AddQueue( h264Stream *stream )
{
	pthread_mutex_lock( &mutex_t );
	cout << "ConnectionThread::AddQueue() is called" << endl;

	if( WaitIFrame ) {
		type = stream->getFrameType();
		if( type == I_SLICE ) {
			WaitIFrame = false;
			Send = true;
		}
		else {
			Send = false;
		}
	}

	//Send = true;
	if( Send ) {
		X = stream->getX();
		Y = stream->getY();
		Width = stream->getWidth();
		Height = stream->getHeight();
		len = stream->getLength();
		//printf( "%d %d %d %dHere!\n", X, Y, Width, Height );
		printf("set len = %d\n", len);
		memcpy( buf1, stream->getData(), len );
		WaitFrame = false;
	}

	cout << "AddQueue Out" << endl;
	pthread_mutex_unlock( &mutex_t );
}

void ConnectionThread::RunThread( int open_port )
{
	cout << "ConnectionThread::RunThread( Socket *feeback ) is called" << endl;

	//FILE *fp;
	//SendFrame
	int done = 0;
	unsigned char data[65536];
	int file_size = 0;
	unsigned char length[4] = {0};
	unsigned char send_buf[256] = {0};
	int seq_num = (-2);
	unsigned char buffer_frame[65536];
	//SendFrame
	int i;
	
	printf( "CT open_port: %d\n", open_port );

	Connection_Management conn2(open_port,1);
	Socket *feeback;

	feeback = conn2.listener();

	if( Arbiter == 0 ) {
		thread_table.push_back( ptr );
	}
	else if( Arbiter == 1 ) {
		thread_table_1.push_back( ptr );
	}
	else {
		fprintf( stderr, "CT: Wrong Arbiter!!!\n" );
	}

	//fp = fopen( "Server.264", "wb" );

	//Socket *feeback=(Socket *) argc;

	//fprintf(stderr, "In Runthread\n" );

	while(1) {

		while( WaitFrame ) {
			;
		}

		//cout << "[1]]" << endl;
		printf( "Thread Line: %d\n", __LINE__ );

		memset( data , 0 , 65535 );
		memset( send_buf , 0 , 255 );
		memset( buffer_frame , 0 ,65535);

		pthread_mutex_lock( &mutex_t );

		//fprintf(stderr, "Out memset Service\n" );
		file_size = len;

		printf( "Line: %d\n", __LINE__ );

		printf( "file_size= %d\n", file_size );

		sprintf( (char *)send_buf , "Num : %10d" , seq_num );

		*(int *)buffer_frame = len;
		*(int *)(buffer_frame+4) = X;
		*(int *)(buffer_frame+8) = Y;
		*(int *)(buffer_frame+12) = Width;
		*(int *)(buffer_frame+16) = Height;

		memcpy(buffer_frame+20,buf1,len);

		if(file_size<30){
			//printf("print sps / pps %d\n", ftell(fp));
			for(i=0; i<file_size; i++){
				if((i&0xF)==0)putchar('\n');
				printf("%2x ", buf1[i]);
			}
			putchar('\n');
		}
		//printf("write to Server.264 : %d\n", ftell(fp));
		//fwrite( buffer_frame+20, 1, len, fp );

		buffer_frame[36] = (seq_num&0x000000ff);
		buffer_frame[37] = ((seq_num&0x0000ff00)>>8);
		buffer_frame[38] = ((seq_num&0x00ff0000)>>16);
		buffer_frame[39] = ((seq_num&0xff000000)>>24);

		length[0] = file_size & 0x000000ff;
		length[1] = ( file_size >> 8 ) & 0x000000ff;
		length[2] = ( file_size >> 16 ) & 0x000000ff;
		length[3] = ( file_size >> 24 ) & 0x000000ff;

		memcpy( send_buf + 16 , length , 4 );

		cout << "try to send data to client" << endl;

		try {
			feeback->outStream().writeBytes( send_buf , 20 );
			feeback->outStream().flush();
			feeback->outStream().writeBytes( (unsigned char *)buffer_frame , file_size+20 );
			feeback->outStream().flush();

		} catch( Exception &e ) {
			cout << e.str() << endl;
			feeback->shutdown();
			delete feeback;
			len = 0;

			//fclose(fp);
			pthread_mutex_lock( &mutex_G );

			if( Arbiter == 0 ) {
				garbage_table.push_back(ptr);
				printf( "Garbage %p Thread.\n", ptr );
			}
			else if( Arbiter == 1 ) {
				garbage_table_1.push_back(ptr);
				printf( "Garbage_1 %p Thread.\n", ptr );
			}
			else {
				printf( "Shit!!!\n" );
			}

			pthread_mutex_unlock( &mutex_G );

			Send = false;
			done = 1;
			//exit(0);
		}


		WaitFrame = true;

		pthread_mutex_unlock( &mutex_t );

		seq_num++;

		if( file_size == 0 ) {
			cout << "go here" << endl;
			done = 1;
		}

		if( done == 1 ) {
			//garbage_table.push_back(ptr);
			break;
		}

		printf("     %d\n",seq_num);
	}

	cout << "=== Send Done ===" << endl;
}

ConnectionThread::~ConnectionThread()
{
	;
}

