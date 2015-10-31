#ifndef _CONNECTION_THREAD_
#define _CONNECTION_THREAD_

#include <deque>
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/network/TcpSocket.h"
#include "./m2gnetwork/network/Socket.h"
#include "./m2gnetwork/rdr/Exception.h"
#include "./m2gnetwork/rdr/types.h"

#include "h264Stream.h"

using namespace network;
using namespace rdr;

class ConnectionThread {
	public:
		ConnectionThread();
		void RunThread( /*Socket *feeback*/ int open_port );
		void AddQueue( h264Stream *stream );
		bool IsFirst( void );
		void SetSend( bool OK );
		void setArbiter( int num );
		void ChangeFirst( void );
		~ConnectionThread();
	private:
		int type;
		int Arbiter;
		bool First;
		bool WaitIFrame;
		int len;
		int X;
		int Y;
		int Width;
		int Height;
		bool Send;
		bool WaitFrame; 
		unsigned char buf1[65536];
		ConnectionThread *ptr;
};

#endif

