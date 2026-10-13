#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 1) {
    fprintf(2, "Usage: pingpong\n");
    exit(1);
  }
  int cha_sang_con[2];
  int con_sang_cha[2];
  char byte = 'a';
  if (pipe(cha_sang_con) < 0) {
    fprintf(2, "pingpong: pipe cha_sang_con failed\n");
    exit(1);
  }
  if (pipe(con_sang_cha) < 0) {
    fprintf(2, "pingpong: pipe con_sang_cha failed\n");
    close(cha_sang_con[0]);
    close(cha_sang_con[1]);
    exit(1);
  }
  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    close(cha_sang_con[0]);
    close(cha_sang_con[1]);
    close(con_sang_cha[0]);
    close(con_sang_cha[1]);
    exit(1);
  }
  if (pid == 0) {
    close(cha_sang_con[1]);
    close(con_sang_cha[0]);
    if (read(cha_sang_con[0], &byte, 1) != 1) {
      fprintf(2, "pingpong: child read failed\n");
      close(cha_sang_con[0]);
      close(con_sang_cha[1]);
      exit(1);
    }
    printf("%d: received ping\n", getpid());
    if (write(con_sang_cha[1], &byte, 1) != 1) {
      fprintf(2, "pingpong: child write failed\n");
      close(cha_sang_con[0]);
      close(con_sang_cha[1]);
      exit(1);
    }
    close(cha_sang_con[0]);
    close(con_sang_cha[1]);
    exit(0);
  } else {
    close(cha_sang_con[0]);
    close(con_sang_cha[1]);
    int err = 0;
    if (write(cha_sang_con[1], &byte, 1) != 1) {
      fprintf(2, "pingpong: parent write failed\n");
      err = 1;
    }
    close(cha_sang_con[1]);
    if (!err) {
      if (read(con_sang_cha[0], &byte, 1) != 1) {
        fprintf(2, "pingpong: parent read failed\n");
        err = 1;
      } else {
        printf("%d: received pong\n", getpid());
      }
    }
    close(con_sang_cha[0]);
    int child_status = 0;
    wait(&child_status);
    if (err || child_status != 0) {
      exit(1);
    }
    exit(0);
  }
}
