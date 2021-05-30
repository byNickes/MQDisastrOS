#include <stddef.h>
#include <assert.h>
#include "disastrOS_resource.h"
#include "pool_allocator.h"

struct MessageQueue_ptr;

typedef struct MessageQueue{
  Resource resource; //MQ extends resource struct
  ListHead messages; //list of messages
  int written; //written messages
  //MessageQueue_ptr ptr; is this useless?
}MessageQueue;

typedef struct Message{
  ListItem list;
  int pid_sender;
  char* message;
}Message;

//initializes MQ allocator for OS
void MessageQueue_init();

//function to add into resource_alloc_func list
Resource* MessageQueue_alloc();

//function to add into resource_free_func list
int MessageQueue_free(Resource* r);

//functions for message

void Message_init();

//this one will be used on write syscall for MQ
Message* Message_alloc(int pid_sender, char* message);

int Message_free(Message* m);

/* is this useless?
typedef struct MessageQueue_ptr{
  ListItem list;
  MessageQueue* mq_ptr;
}MessageQueue_ptr;
*/
