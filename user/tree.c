#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

// Hàm đệ quy duyệt và in cấu trúc cây
// level: mức hiện tại (gốc là 0, mục con trực tiếp là 1, ...)
// maxDepth: giới hạn độ sâu (từ cờ -L depth)
// onlyDir: cờ chỉ in thư mục (từ cờ -d)
void
tree_traverse(char *path, int level, int maxDepth, int onlyDir)
{
  if (level > maxDepth)
    return;

  int fd;
  struct stat st;
  struct dirent de;
  char buf[512], *p;

  if ((fd = open(path, O_RDONLY)) < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {
    close(fd);
    return;
  }

  if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
    fprintf(2, "tree: path too long: %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  p = buf + strlen(buf);
  if (p > buf && *(p - 1) != '/') {
    *p++ = '/';
  }

  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0)
      continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;

    // Không duyệt "." và ".."
    if (strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
      continue;

    struct stat st_child;
    if (stat(buf, &st_child) < 0) {
      continue;
    }

    // Nếu có cờ -d mà entry là file thì bỏ qua
    if (onlyDir && st_child.type != T_DIR)
      continue;

    // In thụt lề theo định dạng cây
    for (int i = 0; i < level - 1; i++) {
      printf("|   ");
    }
    printf("|-- %s\n", p);

    // Nếu là thư mục con và chưa vượt quá maxDepth thì đệ quy tiếp
    if (st_child.type == T_DIR) {
      tree_traverse(buf, level + 1, maxDepth, onlyDir);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int maxDepth = 100000; // Mặc định không giới hạn độ sâu
  int onlyDir = 0;       // Mặc định in cả file và thư mục

  // Phân tích các đối số dòng lệnh: tree [path] [-L depth] [-d]
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      onlyDir = 1;
    } else if (strcmp(argv[i], "-L") == 0) {
      if (i + 1 >= argc) {
        fprintf(2, "tree: option requires an argument -- 'L'\n");
        fprintf(2, "Usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
      i++;
      // Xác thực độ sâu là số nguyên không âm
      for (int j = 0; argv[i][j] != '\0'; j++) {
        if (argv[i][j] < '0' || argv[i][j] > '9') {
          fprintf(2, "tree: invalid depth '%s'\n", argv[i]);
          exit(1);
        }
      }
      maxDepth = atoi(argv[i]);
    } else if (argv[i][0] == '-') {
      fprintf(2, "tree: invalid option -- '%s'\n", argv[i]);
      fprintf(2, "Usage: tree [path] [-L depth] [-d]\n");
      exit(1);
    } else {
      path = argv[i];
    }
  }

  // Kiểm tra đường dẫn gốc có tồn tại không
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "tree: cannot open %s\n", path);
    exit(1);
  }

  struct stat st;
  if (fstat(fd, &st) < 0) {
    fprintf(2, "tree: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }
  close(fd);

  // Nếu cờ -d được bật nhưng path lại là file
  if (onlyDir && st.type != T_DIR) {
    exit(0);
  }

  // In gốc (level 0)
  printf("%s\n", path);

  // Nếu gốc là thư mục và maxDepth >= 1 thì bắt đầu duyệt các mục con (level 1)
  if (st.type == T_DIR && maxDepth >= 1) {
    tree_traverse(path, 1, maxDepth, onlyDir);
  }

  exit(0);
}
