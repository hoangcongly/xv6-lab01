#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXLINE 512

// doc 1 dong tu fd vao buf (bo ky tu '\n' o cuoi)
// tra ve -1 neu het file ma khong doc duoc gi, nguoc lai tra ve do dai dong
// dong nao dai hon max-1 thi chi giu phan dau, phan con lai van doc het de bo qua
int
readline(int fd, char *buf, int max)
{
  int i = 0;
  int got = 0;   // co doc duoc ky tu nao chua
  char c;

  while(read(fd, &c, 1) == 1){
    got = 1;
    if(c == '\n')
      break;
    if(i < max - 1)
      buf[i++] = c;
  }
  buf[i] = 0;

  if(!got)
    return -1;
  return i;
}

int
main(int argc, char *argv[])
{
  char *name1 = 0, *name2 = 0;
  int quiet = 0;
  int i;

  // duyet tham so, -q dat o dau hay cuoi cung duoc
  for(i = 1; i < argc; i++){
    if(strcmp(argv[i], "-q") == 0)
      quiet = 1;
    else if(name1 == 0)
      name1 = argv[i];
    else if(name2 == 0)
      name2 = argv[i];
    else {
      name1 = 0;  // du tham so -> bao loi luon
      break;
    }
  }
  if(name1 == 0 || name2 == 0){
    fprintf(2, "usage: diff file1 file2 [-q]\n");
    exit(1);
  }

  int fd1 = open(name1, O_RDONLY);
  if(fd1 < 0){
    fprintf(2, "diff: cannot open %s\n", name1);
    exit(1);
  }
  int fd2 = open(name2, O_RDONLY);
  if(fd2 < 0){
    fprintf(2, "diff: cannot open %s\n", name2);
    close(fd1);
    exit(1);
  }

  static char line1[MAXLINE], line2[MAXLINE];
  int n1, n2;
  int lineno = 0;
  int differ = 0;

  while(1){
    n1 = readline(fd1, line1, MAXLINE);
    n2 = readline(fd2, line2, MAXLINE);

    // ca 2 file deu het thi dung
    if(n1 < 0 && n2 < 0)
      break;

    lineno++;

    // mot ben het file truoc, hoac ca 2 con nhung noi dung khac nhau
    if(n1 < 0 || n2 < 0 || strcmp(line1, line2) != 0){
      differ = 1;

      // che do -q: chi can biet khac la du, khong can doc tiep
      if(quiet)
        break;

      printf("%s:%d: < %s\n", name1, lineno, n1 < 0 ? "EOF" : line1);
      printf("%s:%d: > %s\n", name2, lineno, n2 < 0 ? "EOF" : line2);
    }
  }

  if(quiet && differ)
    printf("diff: files differ\n");

  // giong het nhau thi khong in gi ca
  close(fd1);
  close(fd2);
  exit(0);
}
