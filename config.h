#ifndef CONFIG_H
#define CONFIG_H    

//--------------------------------------//
// define resolution type
//--------------------------------------//

#define QCIF   0
#define CIF    1
#define D1     2
#define HD720  3
#define HD1080 4 
#define MOVIE  5
#define VS     6    // Video Surveillance

//Arbiter
#include <deque>
#include "ConnectionThread.h"

using  std::deque;
typedef deque< ConnectionThread* > ThreadTable;
extern ThreadTable thread_table;
typedef deque< ConnectionThread* > GarbageTable;
extern GarbageTable garbage_table;

typedef deque< ConnectionThread* > ThreadTable_1;
extern ThreadTable_1 thread_table_1;
typedef deque< ConnectionThread* > GarbageTable_1;
extern GarbageTable_1 garbage_table_1;
//Arbiter

struct frameinfo {
	int num;
	int x;
	int y;
	int w;
	int h;
};
typedef struct frameinfo FrameInfo;
extern FrameInfo FrameID[4]; 

extern pthread_mutex_t mutex_G;
extern pthread_mutex_t mutex_G_1;

extern char IP[20];
extern int  PORT;
extern int MAX_CONN;

#endif

