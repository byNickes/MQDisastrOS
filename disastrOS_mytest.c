#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <string.h>
#include "disastrOS.h"

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=MESSAGE_QUEUE;
  int mode=0;
  int fd=disastrOS_openResource(0,type,mode);
  printf("fd=%d\n", fd);

  if(fd >= 0){
    printf("reading on MQ with fd=%d\n", fd);
    char message[5];
    memset(message, 0, 5);

    int message_read = 0;
    disastrOS_sleep(10);

    message_read = 0;
    int res = DSOS_EMQAGAIN;
    while(1){
      int attempt = 1;
      res = DSOS_EMQAGAIN;
      while(res == DSOS_EMQAGAIN){
        printf("child: trying to read, it's attempt: %d ..\n", attempt);
        res = disastrOS_readMessageQueue(fd, message, 5);
        if(res == DSOS_EMQAGAIN)
          printf("child: MQ on fd=%d was empty so I waited, trying again..\n",fd);
        attempt++;
      }

      assert(res >= 0);
      printf("child: message read is %s and is long %d\n", message, res);
      message_read++;
      printf("child read %d messages\n", message_read);
      if(message_read%10){
        for(int i = 0; i < 1000;i++){}
      }
    }
  }

  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");

  disastrOS_spawn(sleeperFunction, 0);

  printf("I'm spawning my child..\n");

  int type=MESSAGE_QUEUE;
  int mode=DSOS_CREATE;
  printf("mode: %d\n", mode);
  printf("opening resource (and creating if necessary)\n");
  int fd=disastrOS_openResource(0,type,mode);
  printf("opened MQ with fd=%d\n", fd);

  char message[5] = "ciao";

  int message_written = 0;
  /*
  for(int i = 0; i < MAX_MESSAGES_FOR_MQ; i++){
    int res = DSOS_EMQAGAIN;
    int attempt = 1;
    while(res == DSOS_EMQAGAIN){
      printf("parent: trying to write, it's attempt: %d ..\n", attempt);
      res = disastrOS_writeMessageQueue(fd, message, 5);
      if(res == DSOS_EMQAGAIN)
        printf("parent: MQ on fd=%d was full so I waited, trying again..\n",fd);
      attempt++;
    }


    assert(res >= 0);
    printf("parent: written message %s on fd=%d that is long %d\n", message, fd, res);
    message_written++;
  }
  printf("message written by parent are %d\n", message_written);
  */

  disastrOS_spawn(childFunction, 0);

  message_written = 0;
  int res = DSOS_EMQAGAIN;
  while(1){
    int attempt = 1;
    res = DSOS_EMQAGAIN;
    while(res == DSOS_EMQAGAIN){
      printf("trying to write, it's attempt: %d ..\n", attempt);
      res = disastrOS_writeMessageQueue(fd, message, 5);
      if(res == DSOS_EMQAGAIN)
        printf("MQ on fd=%d was full so I waited, trying again..\n",fd);
      attempt++;
    }

    assert(res >= 0);
    printf("written message %s on fd=%d that is long %d\n", message, fd, res);
    message_written++;
    printf("parent wrote %d messages\n", message_written);
    if(message_written%10){
      for(int i = 0; i < 1000;i++){}
    }
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while((pid=disastrOS_wait(0, &retval))>=0){
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d\n",
	   pid, retval);
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
