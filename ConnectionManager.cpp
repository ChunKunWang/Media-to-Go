#include <iostream>
#include <cstdlib>
//#include <pthread.h>

#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/network/TcpSocket.h"
#include "./m2gnetwork/network/Socket.h"
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/rdr/types.h"

#include "ConnectionManager.h"
#include "ConnectionThread.h"
#include "config.h"

//Arbiter
#include "./m2gnetwork/network/Network_API.h"
#include "m2gServer.h"
#include "BufferQueue.h"
#include "Encoder.h"
#include "h264Stream.h"


pthread_mutex_t mutex;

int port_count = 0;

BufferQueue *bufQ;
BufferQueue *bufQ1;
//Arbiter

using namespace network;
using namespace rdr;

using std::cout;
using std::endl;

void *Arbiter1( void *argc );
void *Arbiter2( void *argc );
void *Service( void *argc );
void *SendVideo( void *argc );
void InitFrameID();

void ConnectionManager::Init(void)
{
	cout << "ConnectionManager::Init() is called" << endl;

	InitFrameID();

	pthread_t Arbiter_t1, Arbiter_t2;
	pthread_create( &Arbiter_t1 , NULL, Arbiter1, NULL );
	pthread_create( &Arbiter_t2 , NULL, Arbiter2, NULL );

	Listener();

	while( 1 );

}

void ConnectionManager::setbufQ( BufferQueue *buf0Q )
{
	cout << "ConnectionManager::setbufQ is called" << endl;
	bufQ = buf0Q;
}

void ConnectionManager::setbufQ1( BufferQueue *buf1Q )
{
	cout << "ConnectionManager::setbufQ1 is called" << endl;
	bufQ1 = buf1Q;
}

void ConnectionManager::Listener( void )
{

	PORT = 30000;
	port_count = PORT;
	MAX_CONN = 1024;
	//cout << " ==== Waitting to connect for client === " << endl;
	Connection_Management conn1(PORT,MAX_CONN);

	Socket *feeback[MAX_CONN];
	pthread_t Client_t[MAX_CONN];

	for( int i=0; i < MAX_CONN; i++ ) {
		feeback[i]=conn1.listener();

		cout << "Accept connection" << endl;

		fprintf(stderr, "Connection Service\n" );
		pthread_create( &Client_t[i] , NULL, Service, (void *)& (*feeback[i]) );
	}

	fprintf(stderr, "No Connection Quta\n" );
}

void *Service( void *argc )
{
	//fprintf(stderr, "In Service\n" );
	unsigned char buffer_frame1[20]={0};
	unsigned char buffer_frame2[20]={0};
	unsigned char choice[4];
	unsigned char vedio_port[4];
	unsigned char sendvideo[8];
	int port_num=0;
	int channel;
	int *tmp;
	double param_t=0;
	double *param;

	ConnectionThread *ptr;
	ptr = new ConnectionThread;

	Socket *feeback = (Socket*) argc;

	//Set FrameID Information
	*(int *)buffer_frame1 = FrameID[0].num;
	*(int *)(buffer_frame1+4) = FrameID[0].x;
	*(int *)(buffer_frame1+8) = FrameID[0].y;
	*(int *)(buffer_frame1+12) = FrameID[0].w;
	*(int *)(buffer_frame1+16) = FrameID[0].h;

	*(int *)buffer_frame2 = FrameID[1].num;
	*(int *)(buffer_frame2+4) = FrameID[1].x;
	*(int *)(buffer_frame2+8) = FrameID[1].y;
	*(int *)(buffer_frame2+12) = FrameID[1].w;
	*(int *)(buffer_frame2+16) = FrameID[1].h;
	//Set FrameID Information

	/*Protocol*/
	try {
		feeback->outStream().writeBytes( buffer_frame1, 20);
		feeback->outStream().flush();

		feeback->outStream().writeBytes( buffer_frame2, 20);
		feeback->outStream().flush();

		feeback->inStream().readBytes( choice, 4);
		tmp = (int *) choice;
		channel = *tmp;
		fprintf(stderr,"channel num %d \n",channel);


		port_count++;
		port_num = port_count;

		*(int *)vedio_port = port_num;
		
		*(int *)sendvideo = channel;
		*(int *)(sendvideo+4) = port_num;

		pthread_t sendvedio_t;
		pthread_create( &sendvedio_t , NULL, SendVideo, (void *)& (*sendvideo) );

		feeback->outStream().writeBytes( vedio_port, 4);
		feeback->outStream().flush();
	}
	catch ( Exception &e ) {
		fprintf( stderr, "Frame Information Send Failed.\n" );
		//exit(0);
	}

	while( 1 );

	/*Protocol*/
//	pthread_t sendvedio_t;
//	pthread_create( &sendvedio_t , NULL, SendVedio, (void *)& (*ptr) );

//	ptr->RunThread( feeback );
	//ptr->RunThread( port_count );
}

void *SendVideo( void *argc )
{
	int channel=0, port=0;
	int *tmp_1, *tmp_2;

	unsigned char *infor = (unsigned char *) argc;

	tmp_1 = (int *) infor;
	channel = *tmp_1;
	tmp_2 = (int *) (infor+4);
	port = *tmp_2;

	fprintf( stderr, "Go Multi-port\n" );
	printf( "----->%d, %d<-----\n", channel, port );

	ConnectionThread *ptr;
	ptr = new ConnectionThread;


//	pthread_mutex_lock( &mutex );

	if( channel == 1 ) {
		ptr->setArbiter(0);
		//thread_table.push_back( ptr );
	}
	else if( channel == 2 ) {
		ptr->setArbiter(1);
		//thread_table_1.push_back( ptr );
	}
	else {
		printf( "shit: %d\n", channel );
	}

//	pthread_mutex_unlock( &mutex );

	ptr->RunThread( port );
}

void *Arbiter1( void *argc ) 
{
	cout << "ConnectionManager:Arbiter() is called" << endl;
	int i=0, j=0;
	int temp_x=0, temp_y=0, temp_w=0, temp_h=0;

	h264Stream *stream;

	stream = bufQ->getBitstream();
	
	FrameID[0].num = 1;
	FrameID[0].x = stream->getX();
	FrameID[0].y = stream->getY();
	FrameID[0].w = stream->getWidth();
	FrameID[0].h = stream->getHeight();

	while( 1 ) {

		if(thread_table.size() == 0) {
			sleep(1);
		}
		else {
			//usleep(800);
			stream = bufQ->getBitstream();

			temp_x = stream->getX(); 
			temp_y = stream->getY();
			temp_w = stream->getWidth();
			temp_h = stream->getHeight();

			if( FrameID[0].x != temp_x || FrameID[0].y != temp_y ) {
				FrameID[0].num = 1;
				FrameID[0].x = temp_x;
				FrameID[0].y = temp_y;
				FrameID[0].w = temp_w;
				FrameID[0].h = temp_h;
			}

			for( i = thread_table.size(); i > 0; i-- ) {

				printf( "Thread[ %d ] address : %p\n" , i-1 , thread_table.at(i-1) );
				if( thread_table.size() > 0 ) {
					//printf( "Stream Length: %d\n", stream->getLength() );
					if( thread_table.at(i-1)->IsFirst() ) {
						printf( "Hello~ %d Client!\n", i-1 );
						if( bufQ->getSPS() != NULL && bufQ->getPPS() != NULL ) {	
							thread_table.at(i-1)->SetSend(true);
							thread_table.at(i-1)->AddQueue(bufQ->getSPS());
							usleep(300);
							thread_table.at(i-1)->AddQueue(bufQ->getPPS());
							thread_table.at(i-1)->ChangeFirst();
							printf( "Yes PPS&SPS\n" );
						}
						else {
							printf( "No PPS&SPS\n" );
						}
					}
					else {
						printf( "Not First Send.\n" );
						thread_table.at(i-1)->AddQueue( stream );
					}
				}
			}
		}

		// mutex thread_table
		pthread_mutex_lock( &mutex );
		pthread_mutex_lock( &mutex_G );
		if( thread_table.size() >= garbage_table.size() ) {

			for( i = garbage_table.size(); i > 0; i--  ) {
				printf( "[In loop G Size %d, T size %d]\n"
						,garbage_table.size()
						,thread_table.size());

				for( j = 0; j < thread_table.size(); j++ ) {

					if( garbage_table.at(i-1) == thread_table.at(j) ) {
						//delete ConnectionThread
						printf( "(G %d) (T %d )*********************Garbage!!!\n"
								,garbage_table.size()
								,thread_table.size());
						delete (thread_table.at(j));
						//delete thread_table
						thread_table.erase( thread_table.begin()+j );
						//delete garbage
						garbage_table.pop_back();
						printf( "(G %d) (T %d )**********************Garbage!!!\n"
								,garbage_table.size()
								,thread_table.size());					
						break;
					}
				}
			}
		}
		else {
			cout << "Thread Table Error!" << endl;
		}
		pthread_mutex_unlock( &mutex_G );
		pthread_mutex_unlock( &mutex );
	}
}

void *Arbiter2( void *argc ) 
{
	cout << "ConnectionManager:Arbiter2() is called" << endl;
	int i=0, j=0;
	int temp_x=0, temp_y=0, temp_w=0, temp_h=0;

	h264Stream *stream;

	stream = bufQ1->getBitstream();
	
	FrameID[1].num = 2;
	FrameID[1].x = stream->getX();
	FrameID[1].y = stream->getY();
	FrameID[1].w = stream->getWidth();
	FrameID[1].h = stream->getHeight();

	while( 1 ) {

		if(thread_table_1.size() == 0) {
			sleep(1);
		}
		else {
			//usleep(800);
			stream = bufQ1->getBitstream();

			temp_x = stream->getX(); 
			temp_y = stream->getY();
			temp_w = stream->getWidth();
			temp_h = stream->getHeight();

			if( FrameID[1].x != temp_x || FrameID[1].y != temp_y ) {
				FrameID[1].num = 2;
				FrameID[1].x = temp_x;
				FrameID[1].y = temp_y;
				FrameID[1].w = temp_w;
				FrameID[1].h = temp_h;
			}

			for( i = thread_table_1.size(); i > 0; i-- ) {

				printf( "Thread[ %d ] address : %p\n" , i-1 , thread_table_1.at(i-1) );
				if( thread_table_1.size() > 0 ) {
					//printf( "Stream Length: %d\n", stream->getLength() );
					if( thread_table_1.at(i-1)->IsFirst() ) {
						printf( "Hello~ %d Client!\n", i-1 );
						if( bufQ1->getSPS() != NULL && bufQ1->getPPS() != NULL ) {	
							thread_table_1.at(i-1)->SetSend(true);
							thread_table_1.at(i-1)->AddQueue(bufQ1->getSPS());
							usleep(300);
							thread_table_1.at(i-1)->AddQueue(bufQ1->getPPS());
							thread_table_1.at(i-1)->ChangeFirst();
							printf( "Yes PPS&SPS\n" );
						}
						else {
							printf( "No PPS&SPS\n" );
						}
					}
					else {
						printf( "Not First Send.\n" );
						thread_table_1.at(i-1)->AddQueue( stream );
					}
				}
			}
		}

		// mutex thread_table Arbiter2
		pthread_mutex_lock( &mutex );
		pthread_mutex_lock( &mutex_G );
		if( thread_table_1.size() >= garbage_table_1.size() ) {

			for( i = garbage_table_1.size(); i > 0; i--  ) {
				printf( "[In loop G Size %d, T size %d]\n"
						,garbage_table_1.size()
						,thread_table_1.size());

				for( j = 0; j < thread_table_1.size(); j++ ) {

					if( garbage_table_1.at(i-1) == thread_table_1.at(j) ) {
						//delete ConnectionThread
						printf( "(G %d) (T %d )*********************Garbage!!!\n"
								,garbage_table_1.size()
								,thread_table_1.size());
						delete (thread_table_1.at(j));
						//delete thread_table
						thread_table_1.erase( thread_table_1.begin()+j );
						//delete garbage
						garbage_table_1.pop_back();
						printf( "(G %d) (T %d )**********************Garbage!!!\n"
								,garbage_table_1.size()
								,thread_table_1.size());					
						break;
					}
				}
			}
		}
		else {
			cout << "Thread Table Error!" << endl;
		}
		pthread_mutex_unlock( &mutex_G );
		pthread_mutex_unlock( &mutex );
	}
}

void InitFrameID()
{
	//Initial Frame Information
	for( int i=0; i < 4; i++ ) {
		FrameID[i].num = -1;
		FrameID[i].x = -1;
		FrameID[i].y = -1;
		FrameID[i].w = -1;
		FrameID[i].h = -1;
	}
}

