#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <asm/types.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>


#define OV_ADDR  0x36
#define ROHM_ADDR 0x0E
#define I2C_DEV "/dev/i2c-1"

#define _OP_FIRM_DWNLD  0x80
#define _OP_Periphe_RW  0x82
#define _OP_Memory_RW   0x84
#define _OP_AD_TRNSFER  0x86
#define _OP_COEF_DWNLD  0x88
#define _OP_PrgMem_RD   0x8A
#define _OP_SpecialCMD  0x8C



typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#pragma pack(1)
typedef union {
    uchar i2c_buf[33];
    struct {
        uchar opCode;
        uchar data[32];
    } package;
    struct {
        uchar opCode;
        uchar addr;
        uchar data[31];
    } msg;
} ROHM_DATA;
#pragma pack()

#pragma pack(1)
typedef union {
    uchar i2c_buf[3];
    struct {
        uchar reg_addrH;
        uchar reg_addrL;
        uchar value;
    } package;
} OV_DATA;
#pragma pack()


int open_i2cdev(const char *node, int is_tenbit=0);

// TODO Using I2C_RDWR to  transfer i2c messages
/*
struct i2c_rdwr_ioctl_data {
    struct i2c_msg *msgs;
    __u32 nmsgs;  // nember of i2c_msgs
};
*/
/*
struct i2c_msg {
    __u16 addr;  // i2c addr
    __u16 flags;  // flag 0-write 1-read
    __u16 len;  // msg len
    __u8 *buf;  // pointer to msg data
};
*/

int i2c_transfer(int i2c_fd, ushort i2c_addr, ushort is_read, uchar *buf_write, ushort write_count, uchar *buf_read=nullptr, ushort read_count=0);
#define I2C_MAX_NMSG 1

int ois_file_download(int i2c_fd, const char *file, uchar opcode);



#endif // CONFIG_H
