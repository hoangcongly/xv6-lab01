#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Hàm đệ quy tính dung lượng của file/thư mục (đơn vị: bytes)
// Quy ước: Dung lượng thư mục = tổng dung lượng các file/thư mục con bên trong
//          (bỏ qua kích thước bản thân thư mục).
long
du(char *path, int opt_a, int opt_s, int is_root)
{
  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], *p;
  long total = 0;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "du: cannot open %s\n", path);
    return 0;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "du: cannot stat %s\n", path);
    close(fd);
    return 0;
  }

  // Trường hợp path là file thông thường hoặc thiết bị
  if (st.type == T_FILE || st.type == T_DEVICE) {
    close(fd);
    total = (long)st.size;
    // In nếu là đối số gốc truyền vào, hoặc khi có cờ -a (và không bật -s)
    if (is_root || (opt_a && !opt_s)) {
      printf("%d\t%s\n", (int)total, path);
    }
    return total;
  }

  // Trường hợp path là thư mục
  if (st.type == T_DIR) {
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
      fprintf(2, "du: path too long: %s\n", path);
      close(fd);
      return 0;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    if (p > buf && *(p - 1) != '/') {
      *p++ = '/';
    }

    // Đọc từng mục trong thư mục
    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
      if (de.inum == 0)
        continue;

      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;

      // Bỏ qua "." và ".."
      if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
        continue;

      // Cộng dồn dung lượng của mục con đệ quy
      total += du(buf, opt_a, opt_s, 0);
    }
    close(fd);

    // In dung lượng thư mục:
    // - Nếu có cờ -s: chỉ in duy nhất thư mục gốc
    // - Nếu không có -s: in thư mục theo thứ tự duyệt từ dưới lên (post-order)
    if (opt_s) {
      if (is_root) {
        printf("%d\t%s\n", (int)total, path);
      }
    } else {
      printf("%d\t%s\n", (int)total, path);
    }
  }

  return total;
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int opt_a = 0;
  int opt_s = 0;

  // Xử lý các đối số dòng lệnh: du [path] [-a] [-s]
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        if (argv[i][j] == 'a') {
          opt_a = 1;
        } else if (argv[i][j] == 's') {
          opt_s = 1;
        } else {
          fprintf(2, "du: invalid option -- '%c'\n", argv[i][j]);
          fprintf(2, "Usage: du [path] [-a] [-s]\n");
          exit(1);
        }
      }
    } else {
      path = argv[i];
    }
  }

  du(path, opt_a, opt_s, 1);
  exit(0);
}
