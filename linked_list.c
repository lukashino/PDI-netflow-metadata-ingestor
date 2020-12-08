#include "linked_list.h"

bool compare_packets(packet_t *packet_1, packet_t *packet_2) {
    return strcmp(packet_1->protocol, packet_2->protocol) == 0 &&
           (
                   (
                           packet_1->dst_addr == packet_2->dst_addr &&
                           packet_1->src_addr == packet_2->src_addr &&
                           packet_1->dst_port == packet_2->dst_port &&
                           packet_1->src_port == packet_2->src_port
                   )
                   ||
                   (
                           packet_1->dst_addr == packet_2->src_addr &&
                           packet_1->src_addr == packet_2->dst_addr &&
                           packet_1->dst_port == packet_2->src_port &&
                           packet_1->src_port == packet_2->dst_port
                   )
           );
}

flow_record_t *find_flow_record(flow_record_t *list, packet_t *packet) {
    if (list == NULL) {
        return NULL;
    }

    for (flow_record_t *it = list; it != NULL; it = it->next) {
        if (compare_packets(&it->packet, packet)) {
            return it;
        }
    }

    return NULL;
}

void insert_packet(flow_record_t **list, packet_t packet, ssize_t payload_size) {
    if (*list == NULL) {
        *list = create_flow_record(&packet, payload_size);
        return;
    }
    flow_record_t *entry = find_flow_record(*list, &packet);
    if (entry == NULL) {
        // packet not in list, create new record
        flow_record_t *new_record = create_flow_record(&packet, payload_size);
        for (flow_record_t *it = *list; it != NULL; it = it->next) {
            if (it->next == NULL) {
                it->next = new_record;
                break;
            }
        }
    } else if (entry->record_count < MAX_RECORD_ENTRY_COUNT) {
        // packet in list, increment counter, save payload size and timestamp
        entry->record_count = entry->record_count + 1;
        flow_record_entry_t *new_record_entry = create_flow_record_entry(payload_size);
        for (flow_record_entry_t *it = entry->record; it != NULL; it = it->next) {
            if (it->next == NULL) {
                it->next = new_record_entry;
                break;
            }
        }
    }
}

flow_record_t *create_flow_record(packet_t *data, ssize_t payload_size) {
    flow_record_t *l = malloc(sizeof(flow_record_t));
    if (l != NULL) {
        l->packet = *data;
        l->next = NULL;
        l->record_count = 1;
        l->record = create_flow_record_entry(payload_size);
    }

    return l;
}

flow_record_entry_t *create_flow_record_entry(ssize_t payload_size) {
    flow_record_entry_t *l = malloc(sizeof(flow_record_entry_t));
    if (l != NULL) {
        l->payload_size = payload_size;
        l->time = time(0);
        l->next = NULL;
    }

    return l;
}

void destroy_flow_record(flow_record_t *list,flow_record_t *node) {
    flow_record_t *tmp = list;
    if(tmp == node)
    {
        destroy_flow_record_entries(tmp->record);
        free(tmp);
        return;
    }

    while(tmp->next != node )
        {   if(!tmp->next)
                return;
            tmp=tmp->next;

        }
        tmp->next = node->next;
        destroy_flow_record_entries(node->record);
        free(node);
    
}

void destroy_flow_records(flow_record_t *list) {
    flow_record_t *tmp = list;
    while (tmp) {
        destroy_flow_record_entries(tmp->record);
        flow_record_t *prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
}

void destroy_flow_record_entries(flow_record_entry_t *list) {
    flow_record_entry_t *tmp = list;
    while (tmp) {
        flow_record_entry_t *prev = tmp;
        tmp = tmp->next;
        free(prev);
    }
}

/*
int main(int argc, char **argv) {
    flow_record_t *list = NULL;
    packet_t *packet = malloc(sizeof(packet_t));
    packet->src_port = 25;
    packet->dst_port = 35;
    packet->src_addr = 1111;
    packet->dst_addr = 2222;
    strcpy(packet->protocol, "UDP");
    insert_packet(&list, *packet, 222);

    packet->src_port = 35;
    packet->dst_port = 25;
    packet->src_addr = 2222;
    packet->dst_addr = 1111;
    strcpy(packet->protocol, "UDP");
    insert_packet(&list, *packet, 99);
    for (flow_record_t *it = list; it != NULL; it = it->next) {
        printf("%d %d %s\n", it->record_count, it->packet.src_addr, it->packet.protocol);
    }
    destroy_flow_records(list);
    free(packet);
}*/