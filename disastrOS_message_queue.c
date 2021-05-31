#include "disastrOS_message_queue.h"
#include <stdio.h>

#define MQ_SIZE sizeof(MessageQueue)
#define MQ_MEMSIZE (sizeof(MessageQueue)+sizeof(int))
#define MQ_BUFFER_SIZE MQ_MEMSIZE*MAX_NUM_RESOURCES

#define MAX_MESSAGES_FOR_MQ 128
#define M_SIZE sizeof(Message)
#define M_MEMSIZE (sizeof(Message)+sizeof(int))
#define MAX_TOTAL_MESSAGES MAX_MESSAGES_FOR_MQ*MAX_NUM_RESOURCES
#define M_BUFFER_SIZE M_MEMSIZE*MAX_TOTAL_MESSAGES

static PoolAllocator mq_allocator;
static char mem_buffer_mq[MQ_BUFFER_SIZE];

static PoolAllocator m_allocator;
static char mem_buffer_m[M_BUFFER_SIZE];

void MessageQueue_init(){
  int init = PoolAllocator_init(&mq_allocator, MQ_SIZE, MAX_NUM_RESOURCES, mem_buffer_mq, MQ_BUFFER_SIZE);
  assert(!init);
  Message_init();
  return;
}

Resource* MessageQueue_alloc(){
  MessageQueue* mq=(MessageQueue*) PoolAllocator_getBlock(&mq_allocator); //getting memory for new MQ struct

  if(!mq)
    return NULL;

  mq -> available = 0;
  List_init(&mq -> messages);
  return (Resource*)mq;
}

int MessageQueue_free(Resource* r){
  MessageQueue* mq = (MessageQueue*) r;

  ListItem* message = mq->messages.first;
  int r_dealloc = 0;
  while(message != NULL){
    r_dealloc = Message_free((Message*)message);
    if(r_dealloc < 0)
      return r_dealloc;
    message = message -> next;
  }

  return PoolAllocator_releaseBlock(&mq_allocator, mq);
}

Message* MessageQueue_getFirstMessage(MessageQueue* mq){
  return (Message*)mq -> messages.first;
}

void print_MQ(MessageQueue* mq){
  if(mq == NULL)
    return;

  ListItem* m = mq -> messages.first;
  int i = 0;
  while(m){
    printf("MESSAGGIO %d: %s \n", i, (char*)(((Message*)m)->message));
    i++;
    m = m -> next;
  }
}

//message functions

void Message_init(){
  int init = PoolAllocator_init(&m_allocator, M_SIZE, MAX_TOTAL_MESSAGES, mem_buffer_m, M_BUFFER_SIZE);
  assert(!init);
  return;
}

Message* Message_alloc(int pid_sender, char* message, int m_length){
  Message* m=(Message*) PoolAllocator_getBlock(&m_allocator); //getting memory for new Message struct

  if(!m)
    return NULL;
  if(m_length > MAX_MESSAGE_LENGTH)
    return NULL;

  m -> pid_sender = pid_sender;

  for(int i = 0; i < m_length; i++){
    m->message[i] = message[i];
  }
  m -> length = m_length;
  return m;
}

int Message_free(Message* m){
  return PoolAllocator_releaseBlock(&m_allocator, m);
}
