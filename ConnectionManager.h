#ifndef _CONNECTION_MANAGER_
#define _CONNECTION_MANAGER_

#include "BufferQueue.h"

//void *Arbiter(void); 

class ConnectionManager {
	public:
		//Arbiter
		void Init(void);
		//void * Arbiter(void *argc);
		//void * Arbiter(void);
		//Arbiter
		void Listener( void );
		void setbufQ( BufferQueue *buf0Q );
		void setbufQ1( BufferQueue *buf1Q );
};

#endif

