#include "disastrOS_resource.h"
#include "pool_allocator.h"

#define MAX_MESSAGE_LENGTH 64

struct MessageQueue_ptr;

typedef struct MessageQueue{
  Resource resource; //MQ extends resource struct
  ListHead messages; //list of messages
  ListHead waiting_to_read;
  ListHead waiting_to_write;
  int available; //written messages
}MessageQueue;

typedef struct Message{
  ListItem list;
  int pid_sender;
  int length;
  char message[MAX_MESSAGE_LENGTH];
}Message;

//initializes MQ allocator for OS
void MessageQueue_init();

//function to add into resource_alloc_func list
Resource* MessageQueue_alloc();

//function to add into resource_free_func list
int MessageQueue_free(Resource* r);

//gives first message of mq
Message* MessageQueue_getFirstMessage(MessageQueue* mq);

void print_MQ(MessageQueue* mq);

//functions for message

void Message_init();

//this one will be used on write syscall for MQ
Message* Message_alloc(int pid_sender, char* message, int m_length);

int Message_free(Message* m);
