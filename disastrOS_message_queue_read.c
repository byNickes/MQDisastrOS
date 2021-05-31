#include "disastrOS_message_queue.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_descriptor.h"

void internal_MessageQueue_read(){
  int fd = running -> syscall_args[0];
  char* buf_des = (char*)running -> syscall_args[1];
  int buf_length = running -> syscall_args[2];

  Descriptor* mq_des = DescriptorList_byFd(&running -> descriptors, fd);
  if(mq_des == NULL){
    running -> syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }
  MessageQueue* mq = (MessageQueue*) mq_des -> resource; //here we have MQ where we have to read on
  int m_length = MessageQueue_getFirstMessage(mq)->length;
  if(m_length>buf_length){
    running -> syscall_retvalue = DSOS_EMQBUFFERTOOSHORT;
    return;
  }

  //we are ready to read a message in MQ
  Message* m = (Message*)List_detach(&mq->messages, mq->messages.first);
  for(int i = 0; i<m_length; i++){
    buf_des[i]= m -> message[i];
  }
  mq -> available -= 1;

  int res = Message_free(m);
  if(res < 0){
    running -> syscall_retvalue = res;
    return;
  }

  running->syscall_retvalue=m_length;
  return;
}
