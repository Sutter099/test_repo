#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// #define DEBUG
#ifdef DEBUG
#define debug(fmt, args...) fprintf(2, fmt, ## args)
#else
#define debug(fmt, args...)
#endif

int
check_match(char *name, char *pattern)
{
  // check if contains substring
  char *p = name;
  char *q = pattern;
  while(*p && *q){
    if(*p != *q) {
      ++p;
      continue;
    }
    ++p;
    ++q;
  }

  if(*q) // not match
    return 0;

  return 1;
}

void
find(char *path, char *pattern)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  // // test
  // // cannot consume too much memory in each function stack.
  // // Otherwise, usertrap will fail
  // // insert code below will PASS the test
  // char tmpbuf[112];
  // strcpy(tmpbuf, path);
  //
  // // insert code below FAIL the test
  // char tmpbuf[512];
  // strcpy(tmpbuf, path);

  debug("\npath: %s pattern: %s\n", path, pattern);
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    debug(">>> type file\n");
    if (check_match(path, pattern)) {
      printf("%s\n", path);
    }
    debug("<<< type file end\n");
    break;

  case T_DIR:
    debug(">>> type dir\n");
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }

    if (check_match(path, pattern)) {
      printf("%s\n", path);
    }

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      debug("buf: %s\n", buf);

      if (strcmp(p, ".") && strcmp(p, "..")) {
        debug("to find\n");
        find(buf, pattern);
      }
    }
    debug("<<< type dir end\n");
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    printf("usage: find [directory] [pattern]\n");
    exit(0);
  }

  find(argv[1], argv[2]);

  exit(0);
}
