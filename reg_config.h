#ifndef REG_CONFIG_H
#define REG_CONFIG_H

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


#define CHIP_ADDR  0x36
#define I2C_DEV "/dev/i2c-1"

int iic_read(int fd, char buff[], int addr, int count);

int iic_write(int fd, char buff[], int addr, int count);

#endif // REG_CONFIG_H
