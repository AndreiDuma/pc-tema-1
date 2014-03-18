#include "lib.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define HOST "127.0.0.1"
#define PORT 10001
#define NO_TIMEOUT -1

void receiver(bool (*to_network_layer)(packet *)) {
    init(HOST, PORT);

    FILE *log_file;

    seq_nr frame_expected = 0; // cadrul asteptat
    frame r, s;
    event ev;
    bool finished = false;

    while (!finished) {

        /* posibilitati soseste cadru corect, cadru eronat */
        ev = wait_for_event(NO_TIMEOUT);

        /* obtine cadru*/
        from_physical_layer(&r);

        /* log */
        log_file = fopen(LOG_FILE, "a");
        print_current_time(log_file);
        fprintf(log_file, " [receiver] Am primit urmatorul pachet:\n");
        print_frame(log_file, &r);
        fclose(log_file);

        if (ev == EVENT_CORRECT) { /* cadru valid a sosit*/

            /* log */
            log_file = fopen(LOG_FILE, "a");
            fprintf(log_file,
                    "Am calculat checksum si pachetul este corect. Voi trimite "
                    "ACK pentru Seq No %d (ultimul cadru)\n"
                    "-----------------------------------------------------------------------------------------------\n",
                    frame_expected);
            fclose(log_file);

            /* este ceea ce se astepta */
            if (r.seq == frame_expected) {

                /* paseaza mai departe datele*/
                finished = to_network_layer(&r.payload);

                /* avans contor*/
                inc(&frame_expected);
            }
        } else {
            /* log */
            log_file = fopen(LOG_FILE, "a");
            fprintf(log_file,
                    "Am calculat checksum si am detectat eroare. Voi trimite "
                    "ACK pentru Seq No %d (ultimul cadru corect pe care l-am primit)\n"
                    "-----------------------------------------------------------------------------------------------\n",
                    frame_expected - 1);
            fclose(log_file);
        }

        /* pregateste seq_nr pt ACK*/
        s = make_answer_frame(frame_expected - 1);

        /* log */
        log_file = fopen(LOG_FILE, "a");
        print_current_time(log_file);
        fprintf(log_file, " [receiver] Trimit ACK:\n");
        print_frame(log_file, &s);
        fclose(log_file);

        /* trimite ACK */
        to_physical_layer(&s);
    }
}

bool chunk_processor(packet *p) {
    static int fd = -1;
    static uint32_t bytes_to_receive;

    if (fd < 0) {
        fd = open(destination_filename, O_WRONLY | O_CREAT);
        bytes_to_receive = ((uint32_t *)p->data)[0];
        return false;
    }

    write(fd, p->data, p->length);
    bytes_to_receive -= p->length;

    if (!bytes_to_receive) {
        close(fd);  // close the file
        fd = -1;  // make it ready for a new call
        return true;
    }

    return false;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Bad usage. Try again.\n");
        return 1;
    }

    strncpy(destination_filename, argv[1], 32);
    destination_filename[32] = '\0';

    receiver(chunk_processor);

    return 0;
}
