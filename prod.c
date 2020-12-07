#include "prod.h"

/**
 * Message delivery report callback using the richer rd_kafka_message_t object.
 */
static void msg_delivered(rd_kafka_t *rk,
                          const rd_kafka_message_t *rkmessage, void *opaque) {
    if (rkmessage->err)
        printf("FAILED\n");
        // fprintf(stderr, "%% Message delivery failed (broker %"PRId32"): %s\n", rd_kafka_message_broker_id(rkmessage), rd_kafka_err2str(rkmessage->err));
    else
        printf("DELIVERED\n");
    // fprintf(stderr, "%% Message delivered (%zd bytes, offset %"PRId64", partition %"PRId32", broker %"PRId32"): %.*s\n", rkmessage->len, rkmessage->offset, rkmessage->partition, rd_kafka_message_broker_id(rkmessage), (int)rkmessage->len, (const char *)rkmessage->payload);
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

void sniff_packets() {
    unsigned char *buffer = (unsigned char *) malloc(IP_MAXPACKET);
    int raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (raw_socket < 0) {
        fprintf(stderr, "Socket Error!\n");
        return;
    }
    ssize_t received_data_size;
    while (keepRunning) {
        // receive a packet
        received_data_size = recvfrom(raw_socket, buffer, IP_MAXPACKET, 0, NULL, NULL);
        if (received_data_size < 0) {
            printf("Recvfrom error!\n");
            break;
        }
        process_packet(buffer, received_data_size);
    }
    close(raw_socket);
    free(buffer);
}

void process_packet(unsigned char *buffer, ssize_t size) {
    // get IP header and data after IP header
    struct iphdr *ip_header = (struct iphdr *) buffer;
    unsigned char *ip_data = buffer + ip_header->ihl * 4;
    // insert packet into list
    ssize_t payload = size - ip_header->ihl * 4;
    packet_t *packet = malloc(sizeof(packet_t));
    packet->src_addr = ip_header->saddr;
    packet->dst_addr = ip_header->daddr;

    if (ip_header->protocol == 6) { // TCP
        struct tcphdr *tcp_header = (struct tcphdr *) ip_data;
        strcpy(packet->protocol, "TCP");
        packet->src_port = ntohs(tcp_header->source);
        packet->dst_port = ntohs(tcp_header->dest);
        payload -= tcp_header->doff * 4;
    } else if (ip_header->protocol == 17) { // UDP
        struct udphdr *udp_header = (struct udphdr *) ip_data;
        strcpy(packet->protocol, "UDP");
        packet->src_port = ntohs(udp_header->source);
        packet->dst_port = ntohs(udp_header->dest);
        payload -= sizeof(udp_header);
    }
    insert_packet(&list, *packet, payload);
    free(packet);
}

void intHandler(int dummy) {
    UNUSED(dummy);
    printf("Stopping program...\n");
    keepRunning = false;
}

int main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    char brokers[1024];
    char topic[256];
    char err_str[512];
    char tmp[16];
    char buf[2048];

    signal(SIGINT, intHandler);

    // ****************************
    // ****** KAFKA SETUP *********
    // ****************************
    // KEEP IN MAIN
    strcpy(brokers, KAFKA_SERVER_IP);
    strcpy(topic, KAFKA_TOPIC);

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
    // ****** KAFKA  SEND *********
    // ****************************
    // TODO posielanie listu a nie tohto stringu
    strcpy(buf, "ahoj");
    size_t len = strlen(buf);
    kafka_send(buf, len);

    sniff_packets();
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
