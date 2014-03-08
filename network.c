#include "lib.h"
#include <unistd.h>
#include <fcntl.h>

void chunk_provider(packet *p) {
    static int fd = -1;

    if (fd < 0) {
        fd = open(source_filename, O_RDONLY);
    }

    uint8_t bytes_to_send = random_payload_length();
    p->length = read(fd, p->data, bytes_to_send);
}

void chunk_processor(packet *p) {
    static int fd = -1;

    if (fd < 0) {
        fd = open(source_filename, O_WRONLY | O_CREAT);
    }

    write(fd, p->data, p->length);
}
