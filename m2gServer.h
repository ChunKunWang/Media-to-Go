#ifndef _M2G_SERVER_
#define _M2G_SERVER_

#include "BufferQueue.h"
#include "Encoder.h"
#include "ConnectionManager.h"

#define MAX_ENCODER_NUM 2

class m2gServer {
	public:
		m2gServer();
		void run();
		~m2gServer();
	private:
		ConnectionManager *CM;
		Encoder *enc_unit[MAX_ENCODER_NUM];
		BufferQueue *bufQ[MAX_ENCODER_NUM];
		h264Stream *stream[MAX_ENCODER_NUM];
};
#endif
