#include <stddef.h>
#include <assert.h>
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

  if(mq->available == MAX_MESSAGES_FOR_MQ){

    running->status=Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    List_insert(&mq->waiting_to_write, mq->waiting_to_write.last, (ListItem*) PCBPtr_alloc(running)); //we take note of who is waiting for something to read into MQ struct

    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    next_running -> status = Running;
    running=next_running;
    return;
  }

  //we are ready to write a message in MQ
  Message* m = Message_alloc(pid_sender, message, m_length);

  List_insert(&mq->messages, mq->messages.last, (ListItem*)m);

  while(mq->waiting_to_read.size > 0){
    PCBPtr* put_in_ready = (PCBPtr*)List_detach(&mq->waiting_to_read, mq->waiting_to_read.first); //we remove one from the waiting list to write of the MQ
    List_detach(&waiting_list, (ListItem*)put_in_ready->pcb);

    PCB* pir_pcb = (PCB*) put_in_ready->pcb;
    pir_pcb -> status = Ready;
    pir_pcb -> syscall_retvalue = DSOS_EMQAGAIN;

    List_insert(&ready_list, ready_list.last, (ListItem*)put_in_ready->pcb);

    assert(PCBPtr_free(put_in_ready)>=0);
  }

  mq -> available += 1;
  running -> syscall_retvalue = m_length;
  return;
}
