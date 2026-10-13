#include "kernel/types.h"
#include "user/user.h"

void sieve(int left_fd) {
  while (1) {
    int prime;
    int n = read(left_fd, &prime, sizeof(int));
    if (n == 0) {
      close(left_fd);
      exit(0);
    }
    if (n != sizeof(int)) {
      fprintf(2, "primes: read prime failed\n");
      close(left_fd);
      exit(1);
    }
    printf("prime %d\n", prime);
    int p[2];
    int has_child = 0;
    int num;
    int err = 0;
    while ((n = read(left_fd, &num, sizeof(int))) == sizeof(int)) {
      if (num % prime != 0) {
        if (!has_child) {
          if (pipe(p) < 0) {
            fprintf(2, "primes: pipe failed\n");
            err = 1;
            break;
          }
          int pid = fork();
          if (pid < 0) {
            fprintf(2, "primes: fork failed\n");
            close(p[0]);
            close(p[1]);
            err = 1;
            break;
          }
          if (pid == 0) {
            close(left_fd);
            close(p[1]);
            left_fd = p[0];
            goto next_stage;
          }
          close(p[0]);
          has_child = 1;
        }
        if (write(p[1], &num, sizeof(int)) != sizeof(int)) {
          fprintf(2, "primes: write failed\n");
          err = 1;
          break;
        }
      }
    }
    if (!err && n != 0) {
      fprintf(2, "primes: read failed\n");
      err = 1;
    }
    close(left_fd);
    int child_status = 0;
    if (has_child) {
      close(p[1]);
      wait(&child_status);
    }
    if (err || child_status != 0) {
      exit(1);
    }
    exit(0);
  next_stage:;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(2, "Usage: primes\n");
    exit(1);
  }

  int p[2];
  if (pipe(p) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    close(p[0]);
    close(p[1]);
    exit(1);
  }

  if (pid == 0) {
    close(p[1]);
    sieve(p[0]);
  } else {
    close(p[0]);
    int err = 0;

    for (int i = 2; i <= 280; i++) {
      if (write(p[1], &i, sizeof(int)) != sizeof(int)) {
        fprintf(2, "primes: initial write failed\n");
        err = 1;
        break;
      }
    }

    close(p[1]);

    int child_status = 0;
    wait(&child_status);

    if (err || child_status != 0) {
      exit(1);
    }
    exit(0);
  }

  return 0;
}
