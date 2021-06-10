#include <stddef.h>
#include <assert.h>
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

  if(((Resource*)mq)->type != MESSAGE_QUEUE){
    running -> syscall_retvalue = DSOS_ESYSCALL_NOT_IMPLEMENTED;
    return;
  }

  Message* first_message = MessageQueue_getFirstMessage(mq);
  if(!first_message){
    //here we have to wait for the queue to be full

    running->status=Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    List_insert(&mq->waiting_to_read, mq->waiting_to_read.last, (ListItem*) PCBPtr_alloc(running)); //we take note of who is waiting for something to read into MQ struct

    PCB* next_running= (PCB*) List_detach(&ready_list, ready_list.first);
    next_running->status=Running;
    running=next_running;
    return;
  }


  int m_length = first_message->length;
  if(m_length>buf_length){
    running -> syscall_retvalue = DSOS_EMQBUFFERTOOSHORT;
    return;
  }

  //we are ready to read a message in MQ
  Message* m = (Message*)List_detach(&mq->messages, (ListItem*)first_message);

  for(int i = 0; i<m_length; i++){
    buf_des[i]= m -> message[i];
  }

  assert(Message_free(m)>=0);

  while(mq->waiting_to_write.size > 0){
    PCBPtr* put_in_ready = (PCBPtr*)List_detach(&mq->waiting_to_write, mq->waiting_to_write.first); //we remove one from the waiting list to write of the MQ
    List_detach(&waiting_list, (ListItem*)put_in_ready->pcb);

    PCB* pir_pcb = (PCB*) put_in_ready->pcb;
    pir_pcb -> status = Ready;
    pir_pcb -> syscall_retvalue = DSOS_EMQAGAIN;

    List_insert(&ready_list, ready_list.last, (ListItem*)put_in_ready->pcb);

    assert(PCBPtr_free(put_in_ready)>=0);
  }

  mq -> available -= 1;
  running -> syscall_retvalue = m_length;
  return;
}
