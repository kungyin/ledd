#ifndef _DEF_H_
#define _DEF_H_



#define MAX_STRING_SIZE     1<<8

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

#endif
