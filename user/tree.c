#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


// hàm in thụt lề theo độ sâu của thư mục / file
void print_indent(int level)
{
    for(int i =1; i<level; i++)
    {
        printf("|   ");
    }
    printf("|__ ");

}

// hàm tree
void tree(char *path, int level , int maxDepth, int onlyDir)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if(level > maxDepth)
        return;

    // dùng system call open để mở file/thư mục theo đường dẫn path
    if((fd = open(path, O_RDONLY))<0){ //fd là biến nhận kết quả quả về (nếu là số dương thì nghĩa là mở thành công)
        printf("tree: cannot open %s\n",path);
        return;

    }

    if(fstat(fd,&st) <  0||st.type != T_DIR){ // system call fstat để đọc thông tin(loại, kích thước,  ..) của file/thư mục
        close(fd);// nếu không đọc được mô tả hoặc không phải là thư mục thì đóng file fd để trách rò rỉ vì XV6 giới hạn số file mở cùng lúc
        return;
    }
    //chuẩn bị tiền tố đường dẫn "path/" trong buf để sẵn sàng ghép tên các file con bên dưới
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    
    // vòng lặp duyêt qua toàn bộ thư mục/file con của đường dẫn path
    while(read(fd, &de, sizeof(de)) == sizeof(de))
    {
        // bỏ qua thư mục rác/ đã bị xóa
        if(de.inum == 0)
            continue;

        // bỏ qua thư mục "." và ".." theo yêu cầu của đề bài
        if(strcmp(de.name, ".") ==0 || strcmp(de.name,"..")==0)
            continue;

        // nối tên thư mục/file con vào tên thư muc/file cha đã tạo ở trên để tạo thành đường dẫn đầy đủ
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] =0;

        //đoc thông tin của thư mục/ file con 
        struct stat st_child;
        if(stat(buf, &st_child)<0)
            continue;

        // bỏ qua nếu user chỉ muốn in thư mục mà đây không phải thư mục
        if(onlyDir && st_child.type != T_DIR)
            continue;

        
        print_indent(level);
        printf("%s\n", de.name);

        // nếu hiện tại đang la thư mục thì tiếp tục duyệt đệ quy sâu vào
        if(st_child.type == T_DIR){
            tree(buf, level +1, maxDepth, onlyDir);

        }
        

    }
    close(fd);

}


int main(int argc, char *argv[]){
    char *path = ".";
    int maxDepth =100;
    int onlyDir = 0;

    for(int i =1; i< argc; i++)
    {
        if(strcmp(argv[i], "-d") ==0)
        {
            onlyDir = 1;
        }
        else if(strcmp(argv[i], "-L") == 0 && i +1 < argc)
        {
            maxDepth = atoi(argv[i+1]);
            i++;

        }
        else if(argv[i][0] != '-')
        {
            path = argv[i];
        }
    }
    printf("%s\n", path);

    tree(path, 1, maxDepth, onlyDir);
    
    exit(0);

}
