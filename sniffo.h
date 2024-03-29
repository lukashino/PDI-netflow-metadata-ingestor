#ifndef _PROD_H_
#define _PROD_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <librdkafka/rdkafka.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <stdbool.h>
#include "linked_list.h"

#define KAFKA_SERVER_IP "51.116.188.112:9092"
#define KAFKA_TOPIC "pdi"

#define UNUSED(x) (void)(x)
#define TIMEOUT 20 // seconds

static volatile sig_atomic_t run = 1;
static volatile bool keepRunning = true;

rd_kafka_t *rk;
rd_kafka_topic_t *rkt;
rd_kafka_conf_t *conf;
rd_kafka_topic_conf_t *topic_conf;
rd_kafka_resp_err_t err;

flow_record_t *list = NULL;

static void msg_delivered(rd_kafka_t *rk, const rd_kafka_message_t *rkmessage, void *opaque);

int kafka_send(void *payload, int payload_len);

void sniff_packets();

void process_packet(unsigned char *buffer);

void intHandler(int dummy);

void format_ip(unsigned int ip, char *buffer);

void format_and_send(flow_record_t *temp);

void check_flows();

void *thread_proc(void *arg);



#endif //_PROD_H_
