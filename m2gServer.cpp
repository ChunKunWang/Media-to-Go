#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <malloc.h>
#include <pthread.h>
#include <deque>
#include <sys/wait.h>
#include <linux/limits.h>
#include <iostream>
#include <unistd.h>
#include "config.h"
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/network/TcpSocket.h"
#include "./m2gnetwork/network/Socket.h"
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/rdr/types.h"
#include "./framebuffer/m2g_dev_api.h"
#include "./framebuffer/fbapi.h"
#include "./h264enc/h264enc.h"
#include "./common/common.h"
#include "m2gServer.h"
#include "ConnectionManager.h"
#include "h264Stream.h"
#include "BufferQueue.h"

using namespace network;
using namespace rdr;
using namespace std;

//TcpListener *server;
//Socket *feeback;

m2gServer::m2gServer()
{
	int i;

	CM = new ConnectionManager();

	for(i=0; i<MAX_ENCODER_NUM; i++){
		bufQ[i] = new BufferQueue( 10002 + i * 100 );
	}

	CM->setbufQ( bufQ[0] );
	CM->setbufQ1( bufQ[1] );
}

m2gServer::~m2gServer()
{
	int i;

        for(i=0; i<MAX_ENCODER_NUM; i++){
                delete enc_unit[i];
		delete bufQ[i];
        }
}

void m2gServer::run() {

	pid_t fd;
	pid_t e_fd;
	int i=0;

	// encoder1
	enc_unit[0] = new Encoder( 7901 );
	fd = fork();
	if( fd == 0 ) {		
		while( 1 ) {
			// TODO
			// check shm1
			int status;
			int X;
			int Y;
			int Height;
			int Width;
			int Length;
			unsigned char EncoderFrame[800000];

                        enc_unit[0]->Update();
                        X = enc_unit[0]->getX();
                        Y = enc_unit[0]->getY();
                        Width = enc_unit[0]->getWidth();
                        Height = enc_unit[0]->getHeight();

			if( ( X != -1 ) || ( Y != -1 ) || ( Width != -1 ) || ( Height != -1 ) )
			{
				//sleep(1);
				cout << "EncodeOneFrame is called" << endl;
                                Length = enc_unit[0]->EncodeOneFrame( EncoderFrame );
                                bufQ[0]->addBitstream( EncoderFrame, Length, X, Y, Width, Height );
			}
			// TODO
			// wait encoder1 
			waitpid( fd , &status , WUNTRACED | WCONTINUED );
		}
		exit(0);
	}

	enc_unit[1] = new Encoder( 8901 );
	fd = fork();
	if( fd == 0 ) {		
		while( 1 ) {
			// TODO
			// check shm1
                        int status;
                        int X;
                        int Y;
                        int Height;
                        int Width;
                        int Length;
                        unsigned char EncoderFrame[800000];

                        enc_unit[1]->Update();
                        X = enc_unit[1]->getX();
                        Y = enc_unit[1]->getY();
                        Width = enc_unit[1]->getWidth();
                        Height = enc_unit[1]->getHeight();

                        //printf("%d %d %d %d\n",X,Y,Width,Height);

			if( ( X != -1 ) || ( Y != -1 ) || ( Width != -1 ) || ( Height != -1 ) )
			{
				//sleep(1);
				cout << "EncodeOneFrame is called" << endl;
                                Length = enc_unit[1]->EncodeOneFrame( EncoderFrame);
                                bufQ[1]->addBitstream( EncoderFrame, Length, X, Y, Width, Height );
			}
			// TODO
			// wait encoder1 
			waitpid( fd , &status , WUNTRACED | WCONTINUED );
		}
		exit(0);
	}

        CM->Init();
}

