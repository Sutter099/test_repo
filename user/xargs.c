#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// #define DEBUG
#ifdef DEBUG
#define debug(fmt, args...) fprintf(2, fmt, ## args)
#else
#define debug(fmt, args...)
#endif

int
copy_cmd_args(char *x_argv[], char *argv[], int argc)
{
  int i;
  for (i = 1; i < argc; ++i)
    x_argv[i - 1] = argv[i];

  return i - 1;
}

int
main(int argc, char *argv[])
{
  // read standard input and exec cmd
  char buf[512];
  char *x_argv[16];
  int i, x_argc;
  char *last_pos;

  debug("running xargs\n");

  if (argc < 2) {
    fprintf(2, "at least 1 args\n");
    exit(1);
  }

  x_argc = copy_cmd_args(x_argv, argv, argc);
  last_pos = buf;
  i = 0;
  for (;read(0, buf + i, 1) > 0; ++i) {
    if (buf[i] != '\n' && buf[i] != ' ')
      continue;

    // ' ', '\n'
    debug("pos %d\n", i);
    x_argv[x_argc++] = last_pos;
    last_pos = buf + i + 1;
    if (buf[i] == ' ') {
      debug("read space\n");
      buf[i] = 0; // end of string
      continue;
    }

    buf[i] = 0; // end of string
    if (fork() == 0) { // child
      // print argv and x_argv
      debug("cmd %s\n", argv[1]);
      for (int j = 0; j < x_argc; ++j)
        debug("x_argv%d %s\n", j, x_argv[j]);

      exec(argv[1], x_argv);
    } else { // parent
      wait(0);
      // refresh buf
      x_argc = copy_cmd_args(x_argv, argv, argc);
    }
  }
  exit(0);
}
