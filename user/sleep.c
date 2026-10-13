#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(2, "Usage: sleep ticks\n");
    exit(1);
  }
  char *s = argv[1];
  if (*s == '\0') {
    fprintf(2, "sleep: invalid ticks\n");
    exit(1);
  }
  for (int i = 0; s[i] != '\0'; i++) {
    if (s[i] < '0' || s[i] > '9') {
      fprintf(2, "sleep: invalid ticks '%s'\n", argv[1]);
      exit(1);
    }
  }
  int ticks = atoi(argv[1]);
  if (sleep(ticks) < 0) {
    fprintf(2, "sleep: syscall failed\n");
    exit(1);
  }

  exit(0);
}
