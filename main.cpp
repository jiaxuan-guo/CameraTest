#include "CameraDevice.h"
#include "ImageUtils.h"

int main(int argc, char *argv[]) {
    CameraDevice camera;
    ImageUtils imageUtils;
    int index;
    int count = 1;
    char filename[16];


    if (argc != 2) {
        std::cout << "<Usage> " << argv[0] << " <camera>" << std::endl;
    }

    // init
    camera.init(argv[1]);
    camera.openDevice();
    camera.setFormat();
    camera.requestBuffers();
    camera.mapBuffers();

    // start capture
    for (int i = 0; i < camera.getBufNum(); ++i) {
        camera.queueBuffer(i);
    }
    camera.startCapture();

    while (count--) {
        fd_set fds;
        struct timeval tv;
        int r;
        for (int i = 0; i < camera.getBufNum(); ++i) {
            FD_ZERO(&fds);
            FD_SET(camera.getFd(), &fds);

            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(camera.getFd() + 1, &fds, NULL, NULL, &tv);
            if (r == -1) {
                if (errno == EINTR) {
                    continue;
                }
                printf("select: %s", strerror(errno));
                return -1;
            } else if (r == 0) {
                printf("select: timeout\n");
                return -1;
            } else {
                camera.dequeueBuffer(&index);
                // deal with the buffer
                snprintf(filename, sizeof(filename), "image%d", index);
                imageUtils.saveYUYImage(filename ,camera.getBuffer(index).first, camera.getBuffer(index).second);
                camera.queueBuffer(index);
            }
        }
    }

    camera.stopCapture();
    std::cout << "See you next time~" << std::endl;
    return 0;
}

