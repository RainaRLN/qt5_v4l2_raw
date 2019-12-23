#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#define CAMERA_DEVICE "/dev/video2"
#define PIXELFMT      V4L2_PIX_FMT_SBGGR8
#define CAPTURE_FILE  "bggr8.raw"
#define WIDTH  3264
#define HEIGHT 2448


extern struct buffer *buffer;

extern int fd;
extern struct v4l2_format fmt;
extern struct v4l2_requestbuffers req;
extern unsigned char *bggr;	 /*存放获取的原始视频数据*/
extern unsigned int  uDataLength;  /*存放原始视频数据帧的长度*/
extern unsigned char *bgr;	/*存放转换后的视频数据*/
extern unsigned char *grbg;
extern unsigned int  uIndex;

void open_device();
void get_capabilities();
void enum_fmt();
void try_fmt();
void set_fmt();
void get_fmt();
void init_reqbuf();
void stream_on();
unsigned int get_frame(void **frame_start, unsigned int *len);
void save_picture(char *filename, unsigned char *file_data);
void stream_off();
void exit_failure() __attribute__((noreturn));

#endif // CAPTURE_H
