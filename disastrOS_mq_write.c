#include "disastrOS_message_queue.h"

void internal_mq_write(){
  int pid_sender = running -> pid;
  char* message = running -> syscall_args[0];
  int fd = running -> syscall_args[1];

  return;
}
