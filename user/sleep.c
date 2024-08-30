#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    const char *err_msg = "usage: sleep [seconds]\n";
    write(2, err_msg, strlen(err_msg));
    exit(1);
  }

  const char *msg = "sleep for: ";
  write(1, msg, strlen(msg));
  write(1, argv[1], strlen(argv[1]));
  write(1, "\n", 1);
  sleep(atoi(argv[1]));

  exit(0);
}
