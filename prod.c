#include "prod.h"

/**
 * Message delivery report callback using the richer rd_kafka_message_t object.
 */
static void msg_delivered(rd_kafka_t *rk,
                          const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rk)
        if (opaque)
            printf("msg_delivered");

    if (rkmessage->err) {
        printf("FAILED\n");
        // fprintf(stderr, "%% Message delivery failed (broker %"PRId32"): %s\n", rd_kafka_message_broker_id(rkmessage), rd_kafka_err2str(rkmessage->err));
    } else {
        printf("DELIVERED\n");
        // fprintf(stderr, "%% Message delivered (%zd bytes, offset %"PRId64", partition %"PRId32", broker %"PRId32"): %.*s\n", rkmessage->len, rkmessage->offset, rkmessage->partition, rd_kafka_message_broker_id(rkmessage), (int)rkmessage->len, (const char *)rkmessage->payload);
    }
}

/*
 * Sends data to kafka configured by rkt global variable.
 */
int kafka_send(void *payload, int payload_len) {
    err = RD_KAFKA_RESP_ERR_NO_ERROR;

    if (rd_kafka_produce(
            rkt, RD_KAFKA_PARTITION_UA, RD_KAFKA_MSG_F_COPY,
            /* Payload and length */
            payload, payload_len,
            /* Optional key and its length */
            NULL, 0,
            /* Message opaque, provided in
                * delivery report callback as
                * msg_opaque. */
            NULL) == -1) {
        err = rd_kafka_last_error();
    }

    fprintf(stderr, "%% Sent %d bytes to topic %s partition %i\n",
            payload_len, rd_kafka_topic_name(rkt), RD_KAFKA_PARTITION_UA);


    /* Poll to handle delivery reports */
    rd_kafka_poll(rk, 0);

    return err;
}

void sniff_packets(char *ifr) {
    unsigned char *buffer = (unsigned char *) malloc(IP_MAXPACKET);
    int raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        fprintf(stderr, "Socket Error %d!\n", raw_socket);
        return;
    }
    size_t len = strlen(ifr);
    int rc;
    if(len > 0)
    if ((rc = setsockopt(raw_socket, SOL_SOCKET, SO_BINDTODEVICE, ifr, len)) < 0)
    {
        perror("Setsockopt() error for SO_BINDTODEVICE");
        printf("%s\n", strerror(errno));
        close(raw_socket);
        exit(-1);
    }
    while (keepRunning) {
        // receive a packet
        if (recvfrom(raw_socket, buffer, IP_MAXPACKET, 0, NULL, NULL) < 0) {
            printf("Recvfrom error!\n");
            break;
        }
        process_packet(buffer);
    }
    close(raw_socket);
    free(buffer);
}


void process_packet(unsigned char *buffer) {
    // get IP header and data after IP header without ethernet header
    struct iphdr *ip_header = (struct iphdr *) (buffer + sizeof(struct ethhdr));
    unsigned char *ip_data = buffer + sizeof(struct ethhdr) + ip_header->ihl * 4;
    // insert packet into list
    ssize_t payload = ntohs(ip_header->tot_len) - ip_header->ihl * 4;
    packet_t *packet = malloc(sizeof(packet_t));
    packet->src_addr = ntohl(ip_header->saddr);
    packet->dst_addr = ntohl(ip_header->daddr);
    flow_record_t *inserted = NULL;

    if (ip_header->protocol == IPPROTO_TCP) { // TCP
        struct tcphdr *tcp_header = (struct tcphdr *) ip_data;
        strcpy(packet->protocol, "TCP");
        packet->src_port = ntohs(tcp_header->source);
        packet->dst_port = ntohs(tcp_header->dest);
        payload -= tcp_header->doff * 4;
        inserted = insert_packet(&list, *packet, payload);
    } else if (ip_header->protocol == IPPROTO_UDP) { // UDP
        struct udphdr *udp_header = (struct udphdr *) ip_data;
        strcpy(packet->protocol, "UDP");
        packet->src_port = ntohs(udp_header->source);
        packet->dst_port = ntohs(udp_header->dest);
        payload -= sizeof(udp_header);
        inserted = insert_packet(&list, *packet, payload);
    }
    // send if max packet count is reached
    if (inserted != NULL && inserted->record_count == MAX_RECORD_ENTRY_COUNT) {
        format_and_send(inserted);
    }
    free(packet);
}

void intHandler(int dummy) {
    UNUSED(dummy);
    printf("Stopping program...\n");
    keepRunning = false;
}

void format_ip(unsigned int ip, char *buffer) {
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    snprintf(buffer, 15, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);
}

void format_and_send(flow_record_t *temp) {
    char buffer[2048];
    temp->record_count = MAX_RECORD_ENTRY_COUNT + 1;

    char form_src_ip[15], form_dst_ip[15];
    format_ip((temp->packet).src_addr, form_src_ip);
    format_ip((temp->packet).dst_addr, form_dst_ip);
    snprintf(buffer, 2048, "%s, %s, %s, %d, %d, ", form_src_ip, form_dst_ip, (temp->packet).protocol,
             (temp->packet).src_port, (temp->packet).dst_port);
    flow_record_entry_t *tmp_ent = temp->record;
    while (tmp_ent) {
        char time_stamp[24], buff[50];
        strftime(time_stamp, 24, "%d-%m-%Y %H:%M:%S", localtime(&(tmp_ent->time)));
        if (tmp_ent->next)
            snprintf(buff, 50, "%zd, %s, ", tmp_ent->payload_size, time_stamp);
        else
            snprintf(buff, 50, "%zd, %s", tmp_ent->payload_size, time_stamp);
        strcat(buffer, buff);
        tmp_ent = tmp_ent->next;
    }
    size_t len = strlen(buffer);
    kafka_send(buffer, len);
}

void check_flows() {
    flow_record_t *temp = list;
    while (temp != NULL) {
        flow_record_entry_t *record = temp->record;
        // get to the end of records
        while (record->next != NULL) {
            record = record->next;
        }
        if ((temp->record_count == MAX_RECORD_ENTRY_COUNT) ||
            ((time(0) - record->time > TIMEOUT) && (temp->record_count < MAX_RECORD_ENTRY_COUNT))) {
            format_and_send(temp);
        }
        temp = temp->next;
    }
}

void *thread_proc(void *arg) {
    UNUSED(arg);
    while (keepRunning) {
        sleep(10);
        check_flows();
    }
    return 0;
}

void printHelp() {
    printf("%s\n%s\n%s\n%s\n\n", 
        "    Usage:",
        "      -i <newtork interface>", 
        "      -b <broker>", 
        "      -t <topic>" );
    exit(0);
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    char brokers[1024];
    char topic[256];
    char interface[128];
    char err_str[512];
    char tmp[16];
    int opt;


    signal(SIGINT, intHandler);

    
    // ****************************
    // ****** KAFKA SETUP *********
    // ****************************
    // KEEP IN MAIN
    strcpy(brokers, KAFKA_SERVER_IP);
    strcpy(topic, KAFKA_TOPIC);

    while ((opt = getopt(argc, argv, "hi:t:b:")) != -1) {
        switch (opt) {
        case 'i':
            strcpy(interface, optarg);
            break;
        case 'b':
            strcpy(brokers, optarg);
            break;
        case 't':
            strcpy(topic, optarg);
            break;
        case 'h':
            printHelp();
            break;
        case '?':
            printf("Unknown option `-%c'.\n", optopt);
            printHelp();
            break;
        }
    }


    conf = rd_kafka_conf_new();

    snprintf(tmp, sizeof(tmp), "%i", SIGIO);
    rd_kafka_conf_set(conf, "internal.termination.signal", tmp, NULL, 0);

    /* Topic configuration */
    topic_conf = rd_kafka_topic_conf_new();

    // Callback message for each message sent. You are able to retrieve status whether message was successful
    rd_kafka_conf_set_dr_msg_cb(conf, msg_delivered);

    /* Create Kafka handle */
    if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, err_str, sizeof(err_str)))) {
        fprintf(stderr, "%% Failed to create new producer: %s\n", err_str);
        exit(1);
    }

    if (rd_kafka_brokers_add(rk, brokers) == 0) {
        fprintf(stderr, "%% No valid brokers specified\n");
        exit(1);
    }

    rkt = rd_kafka_topic_new(rk, topic, topic_conf);
    topic_conf = NULL; /* Now owned by topic */

    // ****************************
    // ********* SNIFF ************
    // ****************************
    pthread_t tid;
    pthread_create(&tid, NULL, &thread_proc, NULL);
    sniff_packets(interface);
    destroy_flow_records(list);

    // ****************************
    // ****** KAFKA DESTROY *********
    // ****************************
    if (topic_conf)
        rd_kafka_topic_conf_destroy(topic_conf);

    /* Let background threads clean up and terminate cleanly. */
    run = 5;
    while (run-- > 0 && rd_kafka_wait_destroyed(1000) == -1)
        printf("Waiting for librdkafka to decommission\n");
    if (run <= 0)
        rd_kafka_dump(stdout, rk);


    return 0;
}
