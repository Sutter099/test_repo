#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int fd_parent_send[2];
  int fd_parent_read[2];

  if (pipe(fd_parent_send) < 0 || pipe(fd_parent_read) < 0) {
    fprintf(2, "pingpong: pipe error\n");
    exit(1);
  }

  int pid = fork();
  int self = getpid();
  char to_send = 0x55;
  char to_recv = 0x0;

  // TODO: lock?
  if (pid) {
    // parent
    close(fd_parent_read[1]);
    close(fd_parent_send[0]);
    write(fd_parent_send[1], &to_send, 1);
    read(fd_parent_read[0], &to_recv, 1);
    printf("%d: received pong\n", self);

    close(fd_parent_read[0]);
    close(fd_parent_send[1]);
    wait(0);
  } else {
    // child
    close(fd_parent_read[0]);
    close(fd_parent_send[1]);
    read(fd_parent_send[0], &to_recv, 1);
    printf("%d: received ping\n", self);
    write(fd_parent_read[1], &to_send, 1);

    close(fd_parent_read[1]);
    close(fd_parent_send[0]);
    exit(0);
  }

  exit(0);
}
