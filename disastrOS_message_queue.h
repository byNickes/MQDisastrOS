#include <stddef.h>
#include <assert.h>
#include "disastrOS_resource.h"
#include "pool_allocator.h"

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_BUFFER_SIZE MQ_SIZE*MAX_NUM_RESOURCES

struct MessageQueue_ptr;

typedef struct MessageQueue{
  Resource* resource; //pointer to struct of this MQ into resources list of OS
  ListHead messages; //list of messages
  int written; //written messages
  //MessageQueue_ptr ptr; is this useless?
}MessageQueue;

//initializes MQ for OS
void MessageQueue_init();

//allocates a new MQ resource
MessageQueue* MessageQueue_alloc(Resource* resource, int size);

/* is this useless?
typedef struct MessageQueue_ptr{
  ListItem list;
  MessageQueue* mq_ptr;
}MessageQueue_ptr;
*/
