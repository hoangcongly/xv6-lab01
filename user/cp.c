#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// buffer 512 byte, moi lan doc/ghi toi da chung nay
char buf[512];

int
main(int argc, char *argv[])
{
  int fdsrc, fddst;
  int n;

  // phai co du 2 tham so: file nguon va file dich
  if(argc != 3){
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  // file nguon chi can doc
  fdsrc = open(argv[1], O_RDONLY);
  if(fdsrc < 0){
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  // file dich: chua co thi tao, co roi thi xoa noi dung cu roi ghi de
  fddst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC);
  if(fddst < 0){
    fprintf(2, "cp: cannot open %s\n", argv[2]);
    close(fdsrc);
    exit(1);
  }

  // doc tung khuc tu src roi ghi qua dst, read tra ve 0 la het file
  while((n = read(fdsrc, buf, sizeof(buf))) > 0){
    // ghi khong du n byte la co loi (vd day disk)
    if(write(fddst, buf, n) != n){
      fprintf(2, "cp: write error\n");
      close(fdsrc);
      close(fddst);
      exit(1);
    }
  }

  // read tra ve so am la doc bi loi
  if(n < 0){
    fprintf(2, "cp: read error\n");
    close(fdsrc);
    close(fddst);
    exit(1);
  }

  close(fdsrc);
  close(fddst);
  exit(0);
}
