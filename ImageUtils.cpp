#include "ImageUtils.h"

int ImageUtils::saveYUYImage(char *filename, void* ptr, size_t len) {
    FILE *image_fd;

    image_fd = fopen(filename, "wb");
    fwrite(ptr, len, 1,image_fd);
    return 0;
}