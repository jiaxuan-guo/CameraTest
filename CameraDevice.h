#ifndef __CAMERA_DEVICE__
#define __CAMERA_DEVICE__

#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstring>
#include <sys/mman.h>
#include <iomanip>
#include <vector>
#include <utility>
#include <unistd.h>

class CameraDevice{
public:

    CameraDevice(){}

    ~CameraDevice(){
        close(m_fd);
        unmapBuffers();
    }

    int init(char *dev_name);
    int openDevice();
    int setFormat();
    int requestBuffers();
    int mapBuffers();
    std::pair<void *, size_t> getBuffer(int index);
    int getFd();
    int getBufNum();

    int startCapture();
    int stopCapture();
    int queueBuffer(int index);
    int dequeueBuffer(int *index);


private:
    const char *m_dev_name;
    int m_fd;
	unsigned int m_buf_num = 4;
    std::vector<std::pair<void *, size_t>> m_buffers;  // Remember the buffer length for unmapping later
	// unsigned int m_width;
	// unsigned int m_height;
	// unsigned int m_bytesperline;
	// unsigned int m_imagesize;

    void closeDevice();
    void unmapBuffers();
};


#endif  // #ifndef __CAMERA_DEVICE__