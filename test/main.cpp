
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
int main()
{
    int fd = open("test.txt", O_WRONLY);
    fsync(fd);
    close(fd);
}
