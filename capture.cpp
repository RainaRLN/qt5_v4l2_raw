#include "capture.h"
#include <QDebug>

void open_device()
{
    fd = open(CAMERA_DEVICE, O_RDWR, 0);
    if (fd < 0)
    {
        printf("open %s failed\n\n", CAMERA_DEVICE);
        exit(EXIT_FAILURE);
    }
    //printf("Open %s succeed!\n\n", CAMERA_DEVICE);
    return;
}

void get_capabilities()
{
    struct v4l2_capability cap;
    int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
    if (ret < 0)
    {
        printf("VIDIOC_QUERYCAP failed (%d)\n\n", ret);
        exit_failure();
    }
    printf("-----------VIDIOC_QUERYCAP-------\n");
    printf("    driver:%s\n",cap.driver);
    printf("    card:%s\n",cap.card);
    printf("    bus_info:%s\n",cap.bus_info);
    printf("    version:%08X\n",cap.version);
    printf("    capabilities:%08X\n\n",cap.capabilities);
    return;
}

void enum_fmt()
{
    struct v4l2_fmtdesc fmtdesc;
    fmtdesc.index = 0;
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        printf("------VIDIOC_ENUM_FMT-----\n");
        printf("get the format what the device support\
            \n    pixelformat = %c%c%c%c\
            \n    description = %s\n\n",\
            fmtdesc.pixelformat & 0xFF, (fmtdesc.pixelformat >> 8) & 0xFF, \
            (fmtdesc.pixelformat >> 16) & 0xFF,(fmtdesc.pixelformat >> 24) & 0xFF, \
            fmtdesc.description);
        fmtdesc.index++;
    }
    return;
}

void try_fmt()
{
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = PIXELFMT;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    if (ioctl(fd, VIDIOC_TRY_FMT, &fmt) == -1)  // try
    {
        printf("VIDIOC_TRY_FMT failed!\n\n");
        exit_failure();
    }
    printf("VIDIOC_TRY_FMT succeed!\n");
    printf("width %d, height %d, pixelformat %#x\n\n", \
        fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
    return;
}

void set_fmt()
{
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = PIXELFMT;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)  // set
    {
        printf("VIDIOC_S_FMT failed!\n\n");
        exit_failure();
    }
    //printf("VIDIOC_S_FMT succeed!\n\n");
    //printf("width %d, height %d, pixelformat %#x\n\n", \
    //    fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
    return;
}

void get_fmt()
{
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)  // get
    {
        printf("VIDIOC_G_FMT failed!\n\n");
        exit_failure();
    }
    printf("VIDIOC_G_FMT succeed!\n");
    printf("width %d, height %d, pixelformat %#x\n\n", \
        fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.pixelformat);
    return;
}

void init_reqbuf()
{
    req.count = 20;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1)
    {
        //printf("VIDIOC_REQBUFS map failed!\n\n");
        qDebug() << "VIDIOC_REQBUFS map failed!";
        exit_failure();
    }
    //printf("VIDIOC_REQBUFS map succeed!\n");

    // 查询缓冲区状态
    unsigned int i = 0;
    buffer = (struct buffer *)calloc(req.count, sizeof(*buffer));
    for (i=0; i<req.count; i++)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            printf("VIDIOC_QUERYBUF failed!\n\n");
            exit_failure();
        }
       // printf("VIDIOC_QUERYBUF succeed!\n\n");
        buffer[i].length = buf.length;
        buffer[i].start = mmap(nullptr, buf.length, PROT_READ|PROT_WRITE,\
            MAP_SHARED, fd, buf.m.offset);
        if (buffer[i].start == MAP_FAILED)
        {
            printf("memory map failed!\n\n");
            exit_failure();
        }
        //printf("memory map succeed!\n\n");

        // 将缓存帧加入队列
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
        {
            printf("VIDIOC_QBUF failed!\n\n");
            exit_failure();
        }
        //printf("VIDIOC_QBUF succeed!\
        //    \nFrame buffer%d: address = 0x%p, length = %d\n\n",\
        //    i, buffer[i].start, buffer[i].length);
    }
    return;
}

void stream_on()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
    {
        printf("VIDIOC_STREAMON failed!\n\n");
        exit_failure();
    }
    //printf("VIDIOC_STREAMON succeed!\n\n");
    return;
}

void get_frame(void **frame_start, unsigned int *len)
{
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    {
        printf("VIDIOC_DQBUF failed!\n\n");
        exit_failure();
    }
    //printf("VIDIOC_DQBUF succeed!\n\n");

    *frame_start = buffer[buf.index].start;
    *len = buffer[buf.index].length;

    //uIndex = buf.index;
    //printf("uIndex %d\n", uIndex);

    // 将这一帧放回队列
    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
    {
        printf("VIDIOC_QBUF failed!\n\n");
        exit_failure();
    }
    //printf("VIDIOC_QBUF succeed!\n\n");

    return;
}

void save_picture(char *filename, unsigned char *file_data)
{
    FILE *fp = fopen(filename, "wb");
    if (nullptr == fp)
    {
        printf("Open frame data file failed\n\n");
        exit_failure();
    }
    //printf("Open frame data file succeed!\n\n");
    fwrite(file_data, uDataLength, 1, fp);
    fclose(fp);
    //printf("Save one frame succeed!\n\n");
    return;
}

void stream_off()
{
    enum v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1)
    {
        printf("VIDIOC_STREAMOFF failed!\n\n");
        exit_failure();
    }
    //printf("VIDIOC_STREAMOFF succeed!\n\n");
    return;
}
void exit_failure()
{
    close(fd);
    exit(EXIT_FAILURE);
}
