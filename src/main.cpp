#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <string>

#include "def.h"

using namespace std;

struct EventMask {  
    int        flag;  
    const char *name;  
  
};  

EventMask event_masks[] = {  
       {IN_ACCESS        , "IN_ACCESS"}        ,    
       {IN_ATTRIB        , "IN_ATTRIB"}        ,    
       {IN_CLOSE_WRITE   , "IN_CLOSE_WRITE"}   ,    
       {IN_CLOSE_NOWRITE , "IN_CLOSE_NOWRITE"} ,    
       {IN_CREATE        , "IN_CREATE"}        ,    
       {IN_DELETE        , "IN_DELETE"}        ,    
       {IN_DELETE_SELF   , "IN_DELETE_SELF"}   ,    
       {IN_MODIFY        , "IN_MODIFY"}        ,    
       {IN_MOVE_SELF     , "IN_MOVE_SELF"}     ,    
       {IN_MOVED_FROM    , "IN_MOVED_FROM"}    ,    
       {IN_MOVED_TO      , "IN_MOVED_TO"}      ,    
       {IN_OPEN          , "IN_OPEN"}          ,    

       {IN_DONT_FOLLOW   , "IN_DONT_FOLLOW"}   ,    
       {IN_EXCL_UNLINK   , "IN_EXCL_UNLINK"}   ,    
       {IN_MASK_ADD      , "IN_MASK_ADD"}      ,    
       {IN_ONESHOT       , "IN_ONESHOT"}       ,    
       {IN_ONLYDIR       , "IN_ONLYDIR"}       ,    

       {IN_IGNORED       , "IN_IGNORED"}       ,    
       {IN_ISDIR         , "IN_ISDIR"}         ,    
       {IN_Q_OVERFLOW    , "IN_Q_OVERFLOW"}    ,    
       {IN_UNMOUNT       , "IN_UNMOUNT"}       ,    
};  

void *inotifyThreadFunc(void *);
 
int freadsome(void *dest, size_t remain, FILE *file)  
{  
    char *offset = (char*)dest;  
    while (remain) {  
        int n = fread(offset, 1, remain, file);
        if (n==0) {  
            return -1;  
        }  
  
        remain -= n;  
        offset += n;  
    }  
    return 0;  
}  

void usage()
{

	fprintf(stderr, "ledd, "
		"usage:\n"
        "\n"    
	);
}

void setLedToHigh(int sig)
{
    printf("system(\"echo 0 > /sys/class/gpio/gpio52/value\")\n");
    return;
}

void HandleSigint(int signo, siginfo_t *siginfo, void *none)  
{  
	printf ("Sending PID: %ld, UID: %ld\n",
			(long)siginfo->si_pid, (long)siginfo->si_uid);
    if ((long)siginfo->si_pid == 1) {
        printf("reboot ........................\n");
    }
}

int main(int argc, char *argv[])
{

    pthread_t thread1, thread2; 
    
    char thdata1[128] = "LED1";
    char thdata2[128] = "LED2";
    
    pthread_create (&thread1, NULL, &inotifyThreadFunc, thdata1);
    pthread_create (&thread2, NULL, &inotifyThreadFunc, thdata2);

    struct sigaction act, oact;  
    memset(&act, 0x00, sizeof(struct sigaction));  
    sigemptyset(&act.sa_mask);  
    act.sa_sigaction = HandleSigint;  
    act.sa_flags = SA_SIGINFO;  
    if (sigaction(SIGTERM, &act, &oact) == -1)   
    {     
        perror("sigaction");  
    }  

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

//  const char *target;
//  if (argc == 1) {
//      target = ".";
//  } else {
//      target = argv[1];
//  }

    return 0;
}

/** 
 *  
 *  
**/
void *inotifyThreadFunc( void *ptr )
{
    char *data;            
    data = (char *) ptr;  

    printf("Thread says %s \n", data);
    

    int monitor = inotify_init();  
    if ( -1 == monitor ) {  
        perror("monitor");  
    }  
  
    /* Target path sould be filled. */
    int watcher = inotify_add_watch(monitor, ".", IN_ALL_EVENTS);  
    if ( -1 == watcher  ) {  
        perror("inotify_add_watch");  
    }  
  
    FILE *monitor_file = fdopen(monitor, "r");  
    char last_name[1024];  
    char name[1024];  

    signal(SIGALRM, setLedToHigh);

    /* event:inotify_event -> name:char[event.len] */  
    while (1) {  
        inotify_event event;  
        if ( -1 == freadsome(&event, sizeof(event), monitor_file) ) {  
            perror("freadsome");  
        }  
        if (event.len) {  
            freadsome(name, event.len, monitor_file);  
        } else {  
            sprintf(name, "FD: %d\n", event.wd);  
        }  
  
        if (strcmp(name, last_name) != 0) {  
            puts(name);  
            strcpy(last_name, name);  
        }  
  
        for (int i=0; i<sizeof(event_masks)/sizeof(EventMask); ++i) {  
            if (event.mask & event_masks[i].flag) {  
                printf("\t%s\n", event_masks[i].name);  
            }  
        }  

        cout << "system(\"echo 1 > /sys/class/gpio/gpio52/value\")" << endl;
        ualarm(100000, 0);          
             //0.1 second
    }
     
    pthread_exit(0);
    
}

