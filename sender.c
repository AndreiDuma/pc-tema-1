#include "lib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HOST "127.0.0.1"
#define PORT 10000
#define TIMEOUT 200

void sender(bool (*from_network_layer)(packet *)) {
    init(HOST, PORT);

    FILE *log_file;

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

        /* log */
        log_file = fopen(LOG_FILE, "a");
        print_current_time(log_file);
        fprintf(log_file, " [sender] Am trimis urmatorul pachet:\n");
        print_frame(log_file, &s);
        print_current_time(log_file);
        fprintf(log_file, " [sender] Am pornit cronometrul (timeout este de %dms)\n", TIMEOUT);
        fprintf(log_file, "----------------------------------------------------------------------------\n");
        fclose(log_file);

        /* asteapta producerea unui eveniment (ACK corect, ACK eronat, timeout) */
        ev = wait_for_event(TIMEOUT);

        /* obtine ACK */
        from_physical_layer(&s);

        /* log */
        log_file = fopen(LOG_FILE, "a");
        print_current_time(log_file);
        fprintf(log_file, " [sender] Am primit ACK:\n");
        print_frame(log_file, &s);
        fclose(log_file);

        if (ev == EVENT_CORRECT) {

            if (s.seq == next_frame_to_send) {

                /* obtine urmatorul set de date */
                finished = from_network_layer(&buffer_packet);

                /* avans contor */
                inc(&next_frame_to_send);
            }

            /* log */
            log_file = fopen(LOG_FILE, "a");
            fprintf(log_file,
                    "Am calculat checksum si ACK-ul este corect. Voi trimite "
                    "pachetul cu Seq No %d\n"
                    "-----------------------------------------------------------------------------------------------\n",
                    next_frame_to_send);
            fclose(log_file);
        } else if (ev == EVENT_INCORRECT) {
            /* log */
            log_file = fopen(LOG_FILE, "a");
            fprintf(log_file,
                    "Am calculat checksum si ACK-ul este incorect. Voi trimite "
                    "pachetul cu Seq No %d\n"
                    "-----------------------------------------------------------------------------------------------\n",
                    next_frame_to_send);
            fclose(log_file);
        }

    }
}

bool chunk_provider(packet *p) {
    static int fd = -1;
    struct stat str_stat;

    if (fd < 0) {
        fd = open(source_filename, O_RDONLY);
        fstat(fd, &str_stat);
        p->length = 4;
        ((int *)p->data)[0] = str_stat.st_size;

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
