#include "lib.h"
#include <stdio.h>
#include <string.h>

#define NO_TIMEOUT -1

void receiver(void (*to_network_layer)(packet *)) {
    seq_nr frame_expected = 0; // cadrul asteptat
    frame r, s;
    event ev;

    while (true) {
        /* posibilitati soseste cadru corect, cadru eronat */
        ev = wait_for_event(NO_TIMEOUT);

        if (ev == EVENT_CORRECT) { /* cadru valid a sosit*/

            from_physical_layer(&r); /* obtine cadru*/

            /* este ceea ce se astepta */
            if (r.seq == frame_expected) {
                /* paseaza mai departe datele*/
                to_network_layer(&r.payload);

                /* avans contor*/
                inc(&frame_expected);
            }

            /* pregateste seq_nr pt ACK*/
            s = make_answer_frame(frame_expected - 1);

            /* trimite ACK */
            to_physical_layer(&s);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Bad usage. Try again.");
    }

    strncpy(destination_filename, argv[1], 32);
    destination_filename[32] = '\0';

    receiver(chunk_processor);

    return 0;
}
