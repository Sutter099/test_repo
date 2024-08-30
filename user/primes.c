// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
//
// #define NUM 35
//
// int
// check_prime(int n)
// {
//   int i;
//   for (i = 2; i < n; ++i) {
//     if (n % i == 0) {
//       return 0;
//     }
//   }
//   return 1;
// }
//
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Winfinite-recursion"
// int
// chain(int fd[2], int idx)
// {
//   int fd_next[2];
//
//   close(fd[1]);
//   if (idx <= 0) {
//     uint8 data;
//
//     while (read(fd[0], &data, 1) > 0) {
//       printf("prime %d\n", data);
//     }
//
//     close(fd[0]);
//     exit(0);
//   }
//
//   if (pipe(fd_next) < 0) {
//     fprintf(2, "pipe error\n");
//     exit(1);
//   }
//
//   int pid = fork();
//
//   if (pid) {
//     // parent
//     uint8 data;
//
//     close(fd_next[0]);
//     while (read(fd[0], &data, 1) > 0) {
//       if (check_prime(data)) {
//         // pass prime numbers only
//         write(fd_next[1], &data, 1);
//       }
//     }
//     close(fd_next[1]);
//     close(fd[0]);
//     wait(&pid);
//   } else {
//     // child
//     chain(fd_next, idx - 1);
//   }
//
//   exit(0);
// }
// #pragma GCC diagnostic pop
//
// int
// main(int argc, char *argv[])
// {
//   int fd[2];
//
//   // if (argc != 2) {
//   //   fprintf(2, "usage: primes idx\n");
//   //   exit(1);
//   // }
//   // int idx = atoi(argv[1]);
//   // sqrt(NUM)
//   int idx = 6;
//
//   if (pipe(fd) < 0) {
//     fprintf(2, "pipe error\n");
//     exit(1);
//   }
//
//   int pid = fork();
//   int i;
//
//   if (pid) {
//     // parent
//     // for (i = 2; i <= 35; ++i) {
//     // sleep(10);
//     close(fd[0]);
//     for (i = 2; i <= NUM; ++i) {
//       uint8 data = (uint8)i;
//       write(fd[1], &data, 1);
//     }
//     close(fd[1]);
//   } else {
//     // child
//     chain(fd, idx);
//     exit(0);
//   }
//
//   wait(&pid);
//   exit(0);
// }

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"
void
sieve(int p[2]) {
  int prime, n;
  int next_p[2];

  // Read the first number, which is the prime number
  close(p[1]); // Close write-end of the current pipe
  if (read(p[0], &prime, sizeof(prime)) == 0) {
    // No more numbers to process
    close(p[0]);
    exit(0);
  }

  printf("prime %d\n", prime);

  // Create the next pipe
  if(pipe(next_p) < 0) {
    fprintf(2, "pipe creation failed\n");
    exit(1);
  }

  if(fork() == 0) {
    // Child process
    close(p[0]); // Close read-end of the current pipe in child
    sieve(next_p);
  } else {
    // Parent process
    close(next_p[0]); // Close read-end of the next pipe in parent
    while(read(p[0], &n, sizeof(n)) > 0) {
      if(n % prime != 0) {
        write(next_p[1], &n, sizeof(n));
      }
    }
    close(p[0]);  // Close read-end of the current pipe in parent
    close(next_p[1]); // Close write-end of the next pipe
    wait(0);  // Wait for child to finish
    exit(0);
  }
}
#pragma GCC diagnostic pop

int
main(int argc, char *argv[])
{
  int p[2];
  int i;

  if (pipe(p) < 0) {
    fprintf(2, "pipe creation failed\n");
    exit(1);
  }

  if (fork() == 0) {
    // Child process to start sieve
    sieve(p);
  } else {
    // Parent process sends numbers to sieve
    close(p[0]); // Close read-end of the pipe in parent
    for(i = 2; i <= 35; i++) {
      write(p[1], &i, sizeof(i));
    }
    close(p[1]); // Close write-end after sending all numbers
    wait(0);  // Wait for child to finish
    exit(0);
  }
}
