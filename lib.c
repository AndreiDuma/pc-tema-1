#include "lib.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <limits.h>

frame cached_frame;
int time_to_timeout;

/*
 * Increments the counter given as parameter
 */
void inc(seq_nr *counter) {
    (*counter)++;
}

/*
 * Computes the checksum of frame f
 */
uint8_t compute_checksum(frame f) {
    uint8_t checksum = f.seq;
    uint8_t i;
    for (i = 0; i < f.payload.length; i++) {
        checksum ^= f.payload.data[i];
    }
    return checksum;
}

/*
 * Verifies if the frame's checksum matches the checksum byte
 */
bool verify_checksum(frame f) {
    return f.checksum == compute_checksum(f);
}

/*
 * Creates a frame, given its sequence number and a payload
 * - the checksum is also computed and added
 */
frame make_frame(seq_nr seq, packet *payload) {
    frame f;
    f.seq = seq;
    if (payload != NULL) {
        f.payload.length = payload->length;
        memcpy(f.payload.data, payload->data, payload->length);
    } else {
        f.payload.length = 0;
    }
    f.checksum = compute_checksum(f);

    return f;
}

/*
 * Helper function for creating response frames (no payload)
 */
frame make_answer_frame(seq_nr seq) {
    return make_frame(seq, NULL);
}

/*
 * Constructs a message from a frame
 */
msg frame_to_msg(frame *f) {
    msg m;

    m.len = f->payload.length + 2; /* length part */

    m.payload[0] = f->seq; /* seq part */
    memcpy(m.payload + 1, f->payload.data, f->payload.length); /* payload part */
    m.payload[1 + f->payload.length] = f->checksum; /* checksum part */

    return m;
}

/*
 * Reconstructs a frame out of a message
 */
frame msg_to_frame(msg *m) {
    frame f;

    f.seq = m->payload[0]; /* seq part */
    f.payload.length = m->len - 2; /* length part */
    memcpy(f.payload.data, m->payload + 1, f.payload.length); /* payload part */
    f.checksum = m->payload[1 + f.payload.length]; /* checksum part */

    return f;
}

/*
 * Waits timeout milliseconds to receive a message
 */
event wait_for_event(int timeout) {
    /*
    msg *m = receive_message_timeout(timeout);
    if (m == NULL) {
        return EVENT_TIMEOUT;
    }
    */   // TODO //

    msg r;
    recv_message(&r);
    msg *m = &r;

    cached_frame = msg_to_frame(m);
    if (verify_checksum(cached_frame)) {
        return EVENT_CORRECT;
    }

    return EVENT_INCORRECT;
}

void from_physical_layer(frame *f) {
    *f = cached_frame;
}

void to_physical_layer(frame *f) {
    msg m = frame_to_msg(f);
    send_message(&m);
}

bool is_empty(packet *p) {
    return p->length == 0;
}

/*
 * Returns a payload size between 1 and MAX_PAYLOAD_SIZE
 */
uint8_t random_payload_length(void) {
    srandom(time(NULL));
    return 1 + random() % MAX_PAYLOAD_SIZE;
}

/*
 * Pretty-prints a packet to stdout
 */
void print_packet(packet *p) {
    printf("%d|", p->length);
    uint8_t i;
    for (i = 0; i < p->length; i++) {
        printf("%c", p->data[i]);
    }
    printf("\n");
}

void print_binary(uint8_t byte) {
    uint8_t i,
            mask = 1 << (CHAR_BIT - 1);
    for (i = 0; i < CHAR_BIT; i++) {
        printf("%d", ((byte & mask) == 0) ? 0 : 1);
        mask >>= 1;
    }
}

void print_frame(frame *f) {
    printf("---- FRAME %d ----\n", f->seq);
    print_packet(&f->payload);
    printf("---- ");
    print_binary(f->checksum);
    printf(" ----\n");
}

void print_event(event ev) {
    switch (ev) {
        case EVENT_CORRECT:
            printf("CORRECT FRAME RECEIVED\n");
            break;
        case EVENT_INCORRECT:
            printf("INCORRECT FRAME RECEIVED\n");
            break;
        case EVENT_TIMEOUT:
            printf("TIMEOUT\n");
    }
}
