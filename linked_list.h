#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_RECORD_ENTRY_COUNT 20

typedef struct flow_record_entry {
    time_t time;
    ssize_t payload_size;
    struct flow_record_entry *next;
} flow_record_entry_t;

typedef struct packet {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint16_t src_port;
    uint16_t dst_port;
    char protocol[4];
} packet_t;

typedef struct flow_record {
    packet_t packet;
    flow_record_entry_t *record;
    unsigned int record_count;
    struct flow_record *next;
} flow_record_t;

bool compare_packets(packet_t *packet_1, packet_t *packet_2);

flow_record_t *find_flow_record(flow_record_t *list, packet_t *packet);

void insert_packet(flow_record_t **list, packet_t packet, ssize_t payload_size);

flow_record_t *create_flow_record(packet_t *data, ssize_t payload_size);

flow_record_entry_t *create_flow_record_entry(ssize_t payload_size);

void destroy_flow_record(flow_record_t *list, flow_record_t *node);

void destroy_flow_records(flow_record_t *list);

void destroy_flow_record_entries(flow_record_entry_t *list);

#endif //_LINKED_LIST_H_
