#include "CameraDevice.h"

int CameraDevice::init(char *dev_name) {
    m_dev_name = dev_name;
    m_fd = open(m_dev_name, O_RDWR);
    if (m_fd < 0) {
        throw std::runtime_error("Failed to open device");
    }
    return 0;
}

int CameraDevice::openDevice() {
    struct v4l2_capability cap;
    if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) < 0) {
        std::cout << "Failed to query capability:" << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Driver cap:\t\t" << std::endl
            << "  Driver:\t\t\"" << cap.driver << "\"" << std::endl
            << "  Card:\t\t\t\"" << cap.card << "\"" << std::endl
            << "  Bus:\t\t\t\"" << cap.bus_info << "\"" << std::endl
            << "  Version:\t\t" << ((cap.version >> 16) & 0xFF) << "."
            << ((cap.version >> 8) & 0xFF) << "."
            << (cap.version & 0xFF) << std::endl
            << "  Capabilities:\t\t" << std::hex << std::setw(8) << std::setfill('0') << cap.capabilities << std::dec << std::endl;
    return 0;
}

int CameraDevice::setFormat() {
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = 1920;
    format.fmt.pix.height = 1080;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(m_fd, VIDIOC_S_FMT, &format) < 0) {
        std::cout << "Failed to set format:" << strerror(errno) << std::endl;
    }

    if(ioctl(m_fd, VIDIOC_G_FMT, &format) < 0){
        std::cout << "Failed to get format:" << strerror(errno) << std::endl;
        return -1;
    }

    std::cout << "Camera Format: " << std::endl
    << "  format.type:\t\t" << format.type << std::endl
    << "  pix.pixelformat:\t"
              << static_cast<char>(format.fmt.pix.pixelformat & 0xFF)
              << static_cast<char>((format.fmt.pix.pixelformat >> 8) & 0xFF)
              << static_cast<char>((format.fmt.pix.pixelformat >> 16) & 0xFF)
              << static_cast<char>((format.fmt.pix.pixelformat >> 24) & 0xFF)
              << std::endl
    << "  pix.width:\t\t" << format.fmt.pix.width << std::endl
    << "  pix.height:\t\t" << format.fmt.pix.height << std::endl
    << "  pix.field:\t\t" << format.fmt.pix.field << std::endl;
    return 0;
}

int CameraDevice::requestBuffers() {
    struct v4l2_requestbuffers reqbuf;

    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.count = m_buf_num;
    reqbuf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_REQBUFS, &reqbuf) < 0) {
        std::cout << "Request buffers failed: " << strerror(errno) << std::endl;
    }
    // buffers = calloc(reqbuf.count, sizeof(*buffers));
    printf("Driver allocated %u buffers\n", reqbuf.count);
    return 0;
}

std::pair<void *, size_t> CameraDevice::getBuffer(int index) {
    return m_buffers[index];
}

int CameraDevice::getFd(){
    return m_fd;
}

int CameraDevice::getBufNum(){
    return m_buf_num;
}

int CameraDevice::mapBuffers() {
    struct v4l2_buffer buffer;
    void *mapped;

    for (int i = 0; i < m_buf_num; ++i) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (ioctl(m_fd, VIDIOC_QUERYBUF, &buffer) < 0) {
            std::cout << "Query buffer failed: " << strerror(errno) << std::endl;
        }

        mapped = mmap(
        NULL,
        buffer.length,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        m_fd,
        buffer.m.offset
        );

        if (mapped == MAP_FAILED) {
            std::cout << "mmap failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        m_buffers.push_back(std::make_pair(mapped, buffer.length));
    }
    return 0;
}

int CameraDevice::startCapture() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(m_fd, VIDIOC_STREAMON, &type) < 0) {
        std::cout << "STREAM ON failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraDevice::queueBuffer(int index) {
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = index;
    if (ioctl(m_fd, VIDIOC_QBUF, &buffer) < 0) {
        std::cout << "Queue buffer failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int CameraDevice::dequeueBuffer(int *index) {
    struct v4l2_buffer buffer;

    memset(&buffer, 0, sizeof(buffer));
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    if (ioctl(m_fd, VIDIOC_DQBUF, &buffer) < 0) {
        std::cout << "Dequeue buffer failed: " << strerror(errno) << std::endl;
        return -1;
    }
    *index =  buffer.index;
    return 0;
}

int CameraDevice::stopCapture() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(m_fd, VIDIOC_STREAMOFF, &type) < 0) {
        std::cout << "STREAM OFF failed: " << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

void CameraDevice::unmapBuffers() {
    for (auto &buffer : m_buffers) {
        if (buffer.first) {
            munmap(buffer.first, buffer.second);
        }
    }
    m_buffers.clear();
}
