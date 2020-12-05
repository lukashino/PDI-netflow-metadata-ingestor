#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <librdkafka/rdkafka.h>


static volatile sig_atomic_t run = 1;

static int quiet = 0;
static char errstr[512];
static char tmp[16];

rd_kafka_t *rk;
rd_kafka_topic_t *rkt;
rd_kafka_conf_t *conf;
rd_kafka_topic_conf_t *topic_conf;
rd_kafka_resp_err_t err;

/**
 * Message delivery report callback using the richer rd_kafka_message_t object.
 */
static void msg_delivered (rd_kafka_t *rk,
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

int main (int argc, char **argv) {
	char brokers[1024];
	char topic[256];
	char errstr[512];
	char tmp[16];
    char buf[2048];


    // ****************************
    // ****** KAFKA SETUP *********
    // ****************************
    // KEEP IN MAIN
    strcpy(brokers, "51.116.188.112:9092");
    strcpy(topic, "quickstart-events");

	conf = rd_kafka_conf_new();

    snprintf(tmp, sizeof(tmp), "%i", SIGIO);
	rd_kafka_conf_set(conf, "internal.termination.signal", tmp, NULL, 0);

    /* Topic configuration */
	topic_conf = rd_kafka_topic_conf_new();

    // Callback message for each message sent. You are able to retrieve status whether message was successful
    rd_kafka_conf_set_dr_msg_cb(conf, msg_delivered);

    /* Create Kafka handle */
    if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr)))) {
        fprintf(stderr, "%% Failed to create new producer: %s\n", errstr);
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
    strcpy(buf, "ahoj");
    size_t len = strlen(buf);
    kafka_send(buf, len);


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
