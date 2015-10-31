#include "config.h"

char IP[20];
int PORT;
int MAX_CONN;

//Arbiter
ThreadTable thread_table;
GarbageTable garbage_table;
pthread_mutex_t mutex_G;

ThreadTable_1 thread_table_1;
GarbageTable_1 garbage_table_1;
pthread_mutex_t mutex_G_1;
//Arbiter

FrameInfo FrameID[4]; 


