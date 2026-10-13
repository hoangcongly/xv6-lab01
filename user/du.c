#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"     
#include "kernel/fcntl.h"  


// hàm tính dung lượng đĩa (du - disk usage)
// otp_a: cờ -a (in chi tiết cả file), otp_s: cờ -s (chỉ in tổng kết), is_root: đánh dấu thư mục gốc ban đầu
long du(char *path, int otp_a, int otp_s, int is_root)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    
    // dùng system call open để mở file/thư mục theo đường dẫn path
    if((fd = open(path, O_RDONLY)) < 0)
    {
        printf("du: cannot open %s\n", path);
        return 0;

    }

    // system call fstat để đọc thông tin (loại, kích thước, ...) của file/thư mục
    if(fstat(fd, &st) < 0)
    {
        close(fd); 
        return 0;
    }

    // nếu đối tượng là file thì đóng file và trả về kích thước file
    if(st.type == T_FILE){
        close(fd);

        // Với file là đường dẫn gốc và dùng -s, vẫn phải in tổng của path.
        // Các file con chỉ được in khi có -a.
        if((otp_a && !otp_s) || (is_root && otp_s)){
            printf("%d\t%s\n", (int)st.size, path);
        }
        return st.size;
    }

    if(st.type != T_DIR){
        close(fd);
        return 0;
    }

    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){
    printf("du: path too long\n");   
    close(fd);
    return 0;                           
} 

    // chuẩn bị tiền tố đường dẫn "path/" trong buf để sẵn sàng ghép tên các file con bên dưới
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    long total = 0;

    // vòng lặp duyệt qua toàn bộ thư mục/file con của đường dẫn path
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        // bỏ qua mục tin rác / đã bị xóa
        if(de.inum == 0)
            continue;

        // Nối tên mục con vào đường dẫn cha
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = '\0';

        // Bỏ qua "." và ".."
        if(strcmp(p, ".") == 0 || strcmp(p, "..") == 0)
            continue;
        
        // gọi đệ quy để tính dung lượng của file/thư mục con và cộng dồn vào tổng
        total += du(buf,  otp_a,  otp_s, 0);

    }
    close(fd);

    // nếu không có cờ -s hoặc đây là thư mục gốc ban đầu thì in ra tổng dung lượng
    if(!otp_s || is_root){
        printf("%d\t%s\n", (int)total, path);

    }
    return total;

}

int main(int argc, char *argv[]){
    char *path = ".";
    int otp_a = 0;
    int otp_s = 0;

    for(int i=1; i < argc; i++){
        if(strcmp(argv[i],"-a")==0)
            otp_a = 1;
        else if(strcmp(argv[i],"-s")==0)
            otp_s = 1;
        else if(argv[i][0] == '-'){
            printf("usage: du [path] [-a] [-s]\n");
            exit(1);
        }
        else{
            path = argv[i];
        }
    }

    du(path, otp_a, otp_s, 1);
    exit(0);
}
