#include "reg_config.h"


int iic_write(int fd, char buff[], int addr, int count)
{
    int res;
    int i, n;
    static char sendbuffer[100];
    memcpy(sendbuffer+2, buff, count);
    sendbuffer[0] = addr >> 8;
    sendbuffer[1] = addr;
    res = write(fd, sendbuffer, count+2);
    //printf("write %d byte at 0x%x\n", res, addr);
    return res;
}

int iic_read(int fd, char buff[], int addr, int count)
{
    int res;
    char sendbuffer1[2];
    sendbuffer1[0]=addr >> 8;
    sendbuffer1[1]=addr;
    write(fd, sendbuffer1, 2);
    res = read(fd, buff, count);
    //printf("read %d byte at 0x%x\n", res, addr);
    return res;
}
