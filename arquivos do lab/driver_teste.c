#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "ddriver_L6_ioctl.h"

int main(){
    int fd = open("/dev/muzik", O_RDWR);
    struct conf_st config = {.path = "/root/host_folder"};
    ioctl(fd, CONF_PATH, &config);
    close(fd);
}