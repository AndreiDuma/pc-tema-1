#include "lib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HOST "127.0.0.1"
#define PORT 10000
#define TIMEOUT 1000

void sender(bool (*from_network_layer)(packet *)) {
    init(HOST, PORT);

    seq_nr next_frame_to_send; /* sequence number pentru urmatorul cadru */
    frame s; /* cadru de trimis */
    event ev;
    next_frame_to_send = 0; /* initializare sequence number */

    packet buffer_packet; /* buffer pentru date */
    bool finished = from_network_layer(&buffer_packet); /* citeste primul set de date */

    while (!finished) {

        s = make_frame(next_frame_to_send, &buffer_packet);

        /* trimite cadrul */
        to_physical_layer(&s);

        /* asteapta producerea unui eveniment (ACK corect, ACK eronat, timeout) */
        ev = wait_for_event(TIMEOUT);

//printf("SENDER: ");print_event(ev);

        if (ev == EVENT_CORRECT) {
            from_physical_layer(&s); /* obtine ACK */
//print_frame(&s);
            if (s.seq == next_frame_to_send) {

                /* obtine urmatorul set de date */
                finished = from_network_layer(&buffer_packet);

                /* avans contor */
                inc(&next_frame_to_send);
            }
        }
    }
}

bool chunk_provider(packet *p) {
    static int fd = -1;
    struct stat str_stat;

    if (fd < 0) {
        fd = open(source_filename, O_RDONLY);
        fstat(fd, &str_stat);
        p->length = 1;
        p->data[0] = str_stat.st_size;

        return false;
    }

    uint8_t bytes_to_send = random_payload_length();
    p->length = read(fd, p->data, bytes_to_send);

    if (p->length == 0) {
        close(fd);  // close the file
        fd = -1;  // make it ready for another call
        return true;
    }

    return false;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Bad usage. Try again.\n");
        return 1;
    }

    strncpy(source_filename, argv[1], 32);
    source_filename[32] = '\0';

    sender(chunk_provider);

    return 0;
}
