#include "config.h"
#include <QDebug>


int open_i2cdev(const char *node, int is_tenbit)
{
    int i2c_fd = open(node, O_RDWR);
    if (i2c_fd < 0) {
       qDebug("i2c device open failed");
       return -1;
    }

     // 7bits addr or 10bits addr
    if (ioctl(i2c_fd, I2C_TENBIT, is_tenbit) < 0) {
        qDebug("I2C_TENBIT failed");
        return -1;
    }

    return i2c_fd;
}


int i2c_transfer(int i2c_fd, ushort i2c_addr, ushort is_read, uchar *buf_write, ushort write_count, uchar *buf_read, ushort read_count)
{
    struct i2c_rdwr_ioctl_data rdwr_data;
    struct i2c_msg i2c_data[2];  // TODO struct i2c_msg i2cmsg[I2C_MAX_NMSG];
    int res;

    i2c_data[0].addr = i2c_addr;
    i2c_data[0].flags = 0;
    i2c_data[0].len = write_count;
    i2c_data[0].buf = buf_write;

    i2c_data[1].addr = i2c_addr;
    i2c_data[1].flags = 1;
    i2c_data[1].len = read_count;
    i2c_data[1].buf = buf_read;

    rdwr_data.msgs = i2c_data;
    rdwr_data.nmsgs = is_read + 1;

    res = ioctl(i2c_fd, I2C_RDWR, &rdwr_data);
    if (res < 0) {
        qDebug("i2c_transfer ERROR!");
        return -1;
    }

    return res;
}


int ois_file_download(int i2c_fd, const char *file, uchar opcode)
{
    FILE *fd;
    ROHM_DATA rohm_data;
    int count;
    if ((fd=fopen(file, "rb")) == nullptr) {
        qDebug("Open OIS File failed");
        return -1;
    }

    rohm_data.package.opCode = opcode;
    while (!feof(fd)) {
        count = fread(rohm_data.package.data, 1, 32, fd);
        if (i2c_transfer(i2c_fd, ROHM_ADDR, 0, rohm_data.i2c_buf, count+1) < 0)
            return -1;
    }
    fclose(fd);
    return 0;
}
