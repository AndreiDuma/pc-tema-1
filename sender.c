#include "lib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TIMEOUT 1000

void sender(void (*from_network_layer)(packet *)) {
    seq_nr next_frame_to_send; /* sequence number pentru urmatorul cadru */
    frame s; /* cadru de trimis */
    event ev;
    next_frame_to_send = 0; /* initializare sequence number */

    packet buffer_packet; /* buffer pentru date */
    from_network_layer(&buffer_packet); /* citeste primul set de date */

    while (!is_empty(&buffer_packet)) {

        s = make_frame(next_frame_to_send, &buffer_packet);

        /* seteaza payload */
//        s.payload = buffer;
        /* seteaza dimensiune payload */
//        s.length = length;
        /* seteaza seq_nr */
//        s.seq = next_frame_to_send;

        /* trimite cadrul */
        to_physical_layer(&s);

//
//        /*daca raspunsul dureaza prea mult, time out */
//        start_timer(s.seq);
//

        /* asteapta producerea unui eveniment (ACK corect, ACK eronat, timeout) */
        ev = wait_for_event(TIMEOUT);

        if (ev == EVENT_CORRECT) {
            from_physical_layer(&s); /* obtine ACK */
            if (s.seq == next_frame_to_send) {

//
//                /* opreste timer */
//                stop_timer(s.seq);
//

                /* obtine urmatorul set de date */
                from_network_layer(&buffer_packet);

                /* avans contor */
                inc(&next_frame_to_send);
            }
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Bad usage. Try again.");
    }

    strncpy(source_filename, argv[1], 32);
    source_filename[32] = '\0';

    sender(chunk_provider);

    return 0;
}
