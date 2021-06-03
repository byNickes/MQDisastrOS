#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <string.h>
#include "disastrOS.h"
#include "disastrOS_message_queue.h"

#define MQ 4
#define CHILDREN 8

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction_writer(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=MESSAGE_QUEUE;
  int mode=0;
  int fd_passed = *(int*)args;
  int fd=disastrOS_openResource(fd_passed,type,mode);
  disastrOS_printStatus();

  printf("fd=%d\n", fd);

  char message[MAX_MESSAGE_LENGTH] = "Ciao, sono uno scrittore.";

  int res = DSOS_EMQAGAIN;
  int written_messages = 0;
  for (int i=0; i<MAX_MESSAGES_FOR_MQ; ++i){
    res = DSOS_EMQAGAIN;
    while(res == DSOS_EMQAGAIN){
      res = disastrOS_writeMessageQueue(fd, message, MAX_MESSAGE_LENGTH);
    }

    assert(res >= 0);
    written_messages++;
    printf("child: written message is %s\n\n", message);
  }

  printf("child: written %d messages, I'm exiting..\n", written_messages);
  disastrOS_closeResource(fd);
  disastrOS_exit(written_messages);
}

void childFunction_reader(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=MESSAGE_QUEUE;
  int mode=0;
  int fd_passed = *(int*)args;
  int fd=disastrOS_openResource(fd_passed,type,mode);

  printf("fd_passed vale %d\n", fd_passed);
  disastrOS_printStatus();

  printf("fd=%d\n", fd);

  char message[MAX_MESSAGE_LENGTH];

  int read_messages = 0;
  int res = DSOS_EMQAGAIN;

  for (int i=0; i<MAX_MESSAGES_FOR_MQ; ++i){
    memset(message, 0, sizeof(message));
    res = DSOS_EMQAGAIN;
    while(res == DSOS_EMQAGAIN){
      res = disastrOS_readMessageQueue(fd, message, MAX_MESSAGE_LENGTH);
    }

    assert(res >= 0);
    read_messages++;
    printf("child: read message is %s\n\n", message);
  }
  printf("child: read %d messages, I'm exiting..\n", read_messages);
  disastrOS_closeResource(fd);
  disastrOS_exit(read_messages);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");

  disastrOS_spawn(sleeperFunction, 0);


  printf("I feel like to spawn %d nice threads\n", CHILDREN);
  int alive_children=0;
  int id_resource[MQ];
  int fd[MQ];
  for (int i=0; i<MQ; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    id_resource[i]= i;
    fd[i] = disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd[i]);
  }

  int counter = 0;
  for(int i = 0; i < CHILDREN/2; i++){
    if(counter == MQ) counter = 0;

    disastrOS_spawn(childFunction_writer, id_resource+counter);
    disastrOS_spawn(childFunction_reader, id_resource+counter);

    alive_children+=2;
    counter += 1;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  int read_messages = 0;
  int written_messages = 0;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);

     if(pid%2==0) written_messages += retval;
     else read_messages += retval;

    --alive_children;
  }

  printf("Written messages: %d\n", written_messages);
  printf("Read messages: %d\n", read_messages);
  printf("They must be equal, or there is something wrong!\n");

  for(int i = 0; i < MQ; i++){
    disastrOS_closeResource(id_resource[i]);
    disastrOS_destroyResource(id_resource[i]);
  }

  printf("shutdown!\n");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }

  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
