#ifndef __LIB_H
#define __LIB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "link_emulator/lib.h"

#define MAX_PAYLOAD_SIZE 60
#define LOG_FILE "log.txt"

typedef uint8_t seq_nr;
typedef struct {
    uint8_t data[MAX_PAYLOAD_SIZE];
    uint8_t length;
} packet;

typedef struct {
    seq_nr seq;
    packet payload;
    uint8_t checksum;
} frame;

typedef enum {
    EVENT_CORRECT,
    EVENT_INCORRECT,
    EVENT_TIMEOUT
} event;

char source_filename[32];
char destination_filename[32];

void inc(seq_nr *counter);
uint8_t compute_checksum(frame f);
bool verify_checksum(frame f);
frame make_frame(seq_nr seq, packet *payload);
frame make_answer_frame(seq_nr seq);
msg frame_to_msg(frame *f);
frame msg_to_frame(msg *m);
event wait_for_event(int timeout);
void from_physical_layer(frame *f);
void to_physical_layer(frame *f);
uint8_t random_payload_length(void);
bool is_empty(packet *p);

bool chunk_provider(packet *p);
bool chunk_processor(packet *p);

void print_packet(FILE *stream, packet *p);
void print_binary(FILE *stream, uint8_t byte);
void print_frame(FILE * stream, frame *f);
void print_event(event ev);
void print_current_time(FILE *stream);

#endif
