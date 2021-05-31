#include "disastrOS_message_queue.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_descriptor.h"

void internal_MessageQueue_write(){
  int pid_sender = running -> pid;
  int fd = running -> syscall_args[0];
  char* message = (char*)running -> syscall_args[1];
  int m_length = running -> syscall_args[2];

  if(m_length > MAX_MESSAGE_LENGTH){
    running -> syscall_retvalue = DSOS_EMQMESSAGETOOLONG;
    return;
  }

  Descriptor* mq_des = DescriptorList_byFd(&running -> descriptors, fd);
  if(mq_des == NULL){
    running -> syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }
  MessageQueue* mq = (MessageQueue*)mq_des -> resource; //here we have MQ where we have to write on

  //we are ready to write a message in MQ
  Message* m = Message_alloc(pid_sender, message, m_length);

  List_insert(&mq->messages, mq->messages.last, (ListItem*)m);
  mq -> available += 1;

  running -> syscall_retvalue = m_length;
  return;
}
