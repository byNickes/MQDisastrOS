#include "disastrOS_message_queue.h"

static PoolAllocator mq_allocator;
static char mem_buffer_mq[MQ_BUFFER_SIZE];

void MessageQueue_init(){
  int init = PoolAllocator_init(&mq_allocator, MQ_SIZE, MAX_NUM_RESOURCES, mem_buffer_mq, MQ_BUFFER_SIZE);
  assert(!init);
  return;
}

MessageQueue* MessageQueue_alloc(Resource* resource){
  MessageQueue* mq=(MessageQueue*) PoolAllocator_getBlock(&mq_allocator); //getting memory for new MQ struct
  assert(!mq);

  mq -> resource = resource;
  mq -> written = 0;
  List_init(&mq -> messages);
  return mq;
}

int MessageQueue_free(MessageQueue* mq){
  int r_dealloc = Resource_free(resource);
  if(r_dealloc < 0)
    return r_dealloc;

  ListItem* message = messages.first;
  while(message != NULL){
    r_dealloc = Message_free(messages);
    if(r_dealloc < 0)
      return r_dealloc;
  }

  return PoolAllocator_releaseBlock(&mq_allocator, mq);
}
