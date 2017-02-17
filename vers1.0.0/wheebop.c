/**
* utpd: UDP to TCP proxy daemon
* Copyright (C) 2011
* Angel Ivan Castell Rovira <al004140 at gmail dot com>
*
* This file is part of utpd.
*
* utpd is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* utpd is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with utpd.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ctype.h>
#include <pthread.h>

#include "error.h"
#include "queue.h"

#define SW_VERSION "0.1.0"

#define MAX_PORT_NUMBER 0xFFFF
#define MAX_TIMEOUT 30 * 1000
#define MAX_NUM_THREADS 0xFF
#define MAX_QUEUE_SIZE 0x3FF

#define UDP_FRAME_SIZE 1 * 1024
#define TCP_FRAME_SIZE 4 * 1024

/* Global variables */
const char *g_program_name = 0;

int g_daemon = 0;
int g_verbose = 0;
int g_nr_working_threads = 2;
int g_queue_size = 10;

int g_udp_port = 2011;
int g_udp_timeout = -1; /* Infinite timeout */

int g_tcp_port = 80;
const char *g_tcp_ipv4_addr = "127.0.0.1";
int g_tcp_answer_timeout = 1000;
int g_tcp_conn_retry = 5000;


void
usage_exit()
{
    fprintf(stdout, "UDP to TCP proxy daemon version %s\n",
            SW_VERSION);

    fprintf(stdout, "Usage: %s [-options]\n",
            g_program_name);

    fprintf(stdout, "Options:\n"
            "\t-d run as a Daemon\n"
            "\t-u Udp port\t\t\t\tdefault: %d\n"
            "\t-w number of Working threads\t\tdefault: %d (threads)\n"
            "\t-q connection Queue size (per thread)\tdefault: %d (connections)\n\n"
            "\t-t remote Tcp port\t\t\tdefault: %d\n"
            "\t-i remote Ipv4 address\t\t\tdefault: %s\n"
            "\t-a remote Answer timeout (ms)\t\tdefault: %d (ms)\n"
            "\t-r remote connection Retry (ms)\t\tdefault: %d (ms)\n\n"
            "\t-v increase Verbose level (2 levels)\n"
            "\t-h this Help\n",
            g_udp_port, g_nr_working_threads, g_queue_size, g_tcp_port,
            g_tcp_ipv4_addr, g_tcp_answer_timeout, g_tcp_conn_retry);

    fprintf(stdout, "\nThis software is distributed under the GNU General Public License\n");
    fprintf(stdout, "See the accompanying COPYING file for more details\n");
    exit(1);
}

void
opt_udp_port()
{
    g_udp_port = atoi(optarg);
    if (g_udp_port < 1 || g_udp_port > MAX_PORT_NUMBER)
        error_exit("invalid port value %d. Valid range is [0-%d]",
            g_udp_port,
            MAX_PORT_NUMBER);
}


void
opt_tcp_port()
{
    g_tcp_port = atoi(optarg);
    if (g_tcp_port < 1 || g_tcp_port > MAX_PORT_NUMBER)
        error_exit("invalid port value %d. Valid range is [0-%d]",
            g_tcp_port,
            MAX_PORT_NUMBER);
}

void
opt_tcp_answer_timeout()
{
    g_tcp_answer_timeout = atoi(optarg);
    if (g_tcp_answer_timeout < 0 || g_tcp_answer_timeout > MAX_TIMEOUT)
        error_exit("invalid TCP timeout %d. Valid range is [0-%d]",
            g_tcp_answer_timeout,
            MAX_TIMEOUT);
}

void
opt_tcp_conn_retry()
{
    g_tcp_conn_retry = atoi(optarg);
    if (g_tcp_conn_retry < 0 || g_tcp_conn_retry > MAX_TIMEOUT)
        error_exit("invalid connection retry with TCP timeout %d. Valid range is [0-%d]",
            g_tcp_conn_retry,
            MAX_TIMEOUT);
}

void
opt_tcp_ipv4_addr()
{
    struct sockaddr_in sa;

    g_tcp_ipv4_addr = optarg;

    if (inet_pton(AF_INET, g_tcp_ipv4_addr, &(sa.sin_addr)) == 0)
        error_exit("invalid IPv4 network address format %s",
            g_tcp_ipv4_addr);
}

void
opt_nr_working_threads()
{
    g_nr_working_threads = atoi(optarg);
    if (g_nr_working_threads < 1 || g_nr_working_threads > MAX_NUM_THREADS)
        error_exit("invalid number of working threads %d. Valid range is [1-%d]",
            g_nr_working_threads,
            MAX_NUM_THREADS);
}

void
opt_queue_size()
{
    g_queue_size = atoi(optarg);
    if (g_queue_size < 1 || g_queue_size > MAX_QUEUE_SIZE)
        error_exit("invalid thread connection queue size %d. Valid range is [1-%d]",
            g_queue_size,
            MAX_QUEUE_SIZE);
}

int
open_udp_server_socket()
{
    int s;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == -1)
        error_exit("socket at main: %s", strerror(errno));

    int yes = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        error_exit("setsockopt SO_REUSEADDR=1: %s", strerror(errno));

    struct sockaddr_in sin;
    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(g_udp_port);
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr*) &sin, sizeof(sin)) == -1)
        error_exit("bind at open_udp_server_socket: %s", strerror(errno));

    if (g_verbose > 0)
        fprintf(stdout, "MAIN: UDP server listening on port %d\n", g_udp_port);

    return s;
}

void
read_input_args(int argc, char **argv)
{
    g_program_name = argv[0];

    int opt;
    while ((opt = getopt(argc, argv, "hvdu:w:q:t:i:a:r:")) != -1)
    {
        switch (opt)
        {
            case 'd': { g_daemon = 1; break; }
            case 'u': { opt_udp_port(); break; }
            case 'w': { opt_nr_working_threads(); break; }
            case 'q': { opt_queue_size(); break; }

            case 't': { opt_tcp_port(); break; }
            case 'i': { opt_tcp_ipv4_addr(); break; }
            case 'a': { opt_tcp_answer_timeout(); break; }
            case 'r': { opt_tcp_conn_retry(); break; }

            case 'v': { g_verbose++; break; }
            case 'h': { usage_exit(); break; }
            default : { usage_exit(); }
        }
    }

    if (argc - optind != 0)
        usage_exit();
}

/*
 * Ensures that all bytes on the 'buf' buffer are sent to the
 * UDP client, even if 'buf' is too much long for the kernel
 * to be sent in only one system call.
*/
int
sendto_udp_safe(int sd, unsigned char * buf, int len, int flags, struct sockaddr * in, socklen_t inlen)
{
    int total = 0; /* total number of bytes sent */
    int bytesleft = len; /* bytes still not sent */
    int n;

    while (total < len)
    {
        n = sendto(sd, buf+total, bytesleft, flags, in, inlen);
        if (n == -1) {
            error("sendto_udp_safe error: %s", strerror(errno));
            break;
        }
        total += n;
        bytesleft -= n;
    }

    return (n == -1) ? -1 : 0; /* return -1 on sendto failure, 0 on success */
}


/*
 * Ensures that all bytes on the 'buf' buffer are sent to the
 * TCP server, even if 'buf' is too much long for the kernel
 * to be sent in only one system call. */
int
send_tcp_safe(int sd, char * buf, int len, int flags)
{
    int total = 0; /* total number of bytes sent */
    int bytesleft = len; /* bytes still not sent */
    int n;

    while (total < len)
    {
        n = send(sd, buf+total, bytesleft, flags);
        if (n == -1) {
            error("send_tcp_safe error: %s", strerror(errno));
            break;
        }
        total += n;
        bytesleft -= n;
    }

    return (n == -1) ? -1 : 0; /* return -1 on send failure, 0 on success */
}

int
reconnect_tcp_server(struct sockaddr_in * servaddr)
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == -1)
        error_exit("tcp socket at reconnect_tcp_server: %s", strerror(errno));

    while (-1 == connect(s, (struct sockaddr*)servaddr, sizeof(struct sockaddr))) {
        if (g_verbose > 1)
            fprintf(stdout, "THR: thread sleeping for %d ms before retrying connection\n",
                g_tcp_conn_retry);
        usleep(g_tcp_conn_retry * 1000);
    }

    if (g_verbose > 0)
        fprintf(stdout, "THR: connected with remote TCP server: %s:%d\n",
            g_tcp_ipv4_addr,
            g_tcp_port);

    return s;
}

void *
consumer_main(void *own_queue)
{
    /* Pointer to my own queue */
    struct queue * q;
    q = (struct queue *)own_queue;

    /* Pointer to the current item consumed */
    struct queue_item * consumed_item;

    /* Address of the TCP server to connect to */
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(g_tcp_port);
    inet_pton(AF_INET, g_tcp_ipv4_addr, &servaddr.sin_addr);

    /* Buffer to store the response from servaddr */
    unsigned char response[TCP_FRAME_SIZE]; /* TODO is the allocated space enough? */

    /* Structure used to poll the socket descriptor connected with remote TCP server */
    struct pollfd pfd[1];
    pfd[0].events = POLLIN; /* Interested in input events */

    /* Wait until the first item is enqueued */
    pthread_mutex_lock(&q->mux);
    pthread_cond_wait(&q->not_empty, &q->mux);
    pthread_mutex_unlock(&q->mux);

    while (1)
    {
        /* get next consumed item */
        consumed_item = queue_tail(q);
        if (g_verbose > 1)
            fprintf(stdout, "THR: item consumed from my own queue is: %s\n", consumed_item->frame);

        int tsd = reconnect_tcp_server(&servaddr);

        /* poll input events on tsd */
        pfd[0].fd = tsd;

        /* send answer to TCP server */
        if (-1 == send_tcp_safe(tsd, consumed_item->frame, consumed_item->frame_len, 0))
        {
            /* Close bidirectional tcp socket */
            shutdown(tsd, SHUT_RDWR);
            close(tsd);

            /* Delete connection from my queue */
            queue_tail_del(q);
            continue;
        }

        /* Poll the TCP server socket descriptor for the response */
        int rv = poll(pfd, 1, g_tcp_answer_timeout);

        /* poll error */
        if (rv == -1) {
            error_exit("syscall poll %s", strerror(errno));
        }

        /* poll timeout */
        else if (rv == 0) {
            if (g_verbose > 1)
                fprintf(stdout, "THR: response timeout expired, connection discarted\n");

            /* Close bidirectional tcp socket */
            shutdown(tsd, SHUT_RDWR);
            close(tsd);

            /* Delete connection from my queue */
            queue_tail_del(q);
            continue;
        }

        /* poll data ready */
        else if (pfd[0].revents & POLLIN) {
            if (g_verbose > 0)
                fprintf(stdout, "THR: response received from TCP server\n");

            /* receive response from TCP server */
            bzero(response, TCP_FRAME_SIZE * sizeof(char));
            int rb = recv(tsd, &response, TCP_FRAME_SIZE, 0);

            if (rb < 0) {
                error_exit("message received from the TCP socket failed!\n");
            } else if (rb == 0) { // peer shutdown
                if (g_verbose > 0)
                    fprintf(stdout, "THR: connection closed by peer\n");
            } else if (rb > 0) { /* valid data */
                if (g_verbose > 0)
                    fprintf(stdout, "THR: sending response back to UDP client: %s\n", response);

                /* Create UDP socket */
                int usd = socket(AF_INET, SOCK_DGRAM, 0);
                if (usd == -1)
                    error_exit("udp socket at consumer_main: %s", strerror(errno));

struct sockaddr_in sin;
bzero(&sin, sizeof(struct sockaddr_in));
sin.sin_family = AF_INET;
sin.sin_port = htons(g_udp_port);
sin.sin_addr.s_addr = htonl(INADDR_ANY);

if (bind(usd, (struct sockaddr*) &sin, sizeof(sin)) == -1)
    error_exit("bind at consumer_main: %s", strerror(errno));

                /* Send response to the client */
                sendto_udp_safe(usd, response, rb, 0,
                                (struct sockaddr *)consumed_item->from, *consumed_item->fromlen);

                /* Close UDP socket */
                shutdown(usd, SHUT_RDWR);
                close(usd);
            }

            /* Close bidirectional tcp socket */
            shutdown(tsd, SHUT_RDWR);
            close(tsd);

            /* Delete connection from my queue */
            queue_tail_del(q);
        } /* POLLIN */
    } /* while */
}

int
main(int argc, char **argv)
{
    int i;

    /* Read input arguments */
    read_input_args(argc, argv);

    if (g_verbose > 1) {
        fprintf(stdout, "g_daemon = %d\n", g_daemon);
        fprintf(stdout, "g_udp_port = %d\n", g_udp_port);
        fprintf(stdout, "g_nr_working_threads = %d\n", g_nr_working_threads);
        fprintf(stdout, "g_queue_size = %d\n", g_queue_size);

        fprintf(stdout, "g_tcp_port = %d\n", g_tcp_port);
        fprintf(stdout, "g_tcp_ipv4_addr = %s\n", g_tcp_ipv4_addr);
        fprintf(stdout, "g_tcp_answer_timeout = %d\n", g_tcp_answer_timeout);
        fprintf(stdout, "g_tcp_conn_retry = %d\n", g_tcp_conn_retry);

        fprintf(stdout, "g_verbose = %d\n", g_verbose);
    }

    /* Daemonize server */
    if (g_daemon)
    {
        umask(0);

        int pid = fork();
        if (pid < 0)
            error_exit("%s", strerror(errno));
        else if (pid != 0)
            return 0;

        g_verbose = 0;
        setsid();
        chdir("/");

        close(0);
        close(1);
        close(2);

        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }

    /* Number of read bytes */
    int rb;

    /* UDP client IP, port, size and frame requested */
    struct sockaddr_in * from;
    socklen_t * fromlen;

    /* UDP server socket descriptor, local to the main thread  */
    int ussd = open_udp_server_socket();

    /* UDP server socket poll'ed */
    struct pollfd pfds[1];
    pfds[0].fd = ussd;
    pfds[0].events = POLLIN; /* Input event */

    /* Queue for each thread */
    struct queue * queues;
    queues = (struct queue *) malloc(g_nr_working_threads * sizeof(struct queue));
    if (queues == NULL)
        error_exit("Memory allocation failed!\n");

    for (i = 0; i < g_nr_working_threads; i++)
        queue_new(&queues[i], g_queue_size);

    /* Index of the next queue used to store the data of a new connection */
    int sel_queue = 0;

    /* Set of threads */
    pthread_t * consumers;
    consumers = (pthread_t *) malloc(g_nr_working_threads * sizeof(pthread_t));
    if (consumers == NULL)
        error_exit("Memory allocation failed!\n");

    for (i = 0; i < g_nr_working_threads; i++)
        pthread_create(&consumers[i], NULL, consumer_main, &queues[i]);


    // TODO it would be nice to manage some signal to break out this while
    while (1)
    {
        int rv = poll(pfds, 1, g_udp_timeout);

        /* poll error */
        if (rv == -1) {
            error_exit("syscall poll %s", strerror(errno));
        }

       /* poll timeout */
        else if (rv == 0) {
            if (g_verbose > 1)
                fprintf(stdout, "timeout expired\n");

        /* poll data ready */
        } else {

            if (pfds[0].revents & POLLIN) {
                if (g_verbose > 0)
                    fprintf(stdout, "MAIN: client request received\n");

                /* allocate resources for the client request */
                if (g_verbose > 1)
                    fprintf(stdout, "MAIN: allocating resources for the new request\n");

                char * req = (char *) malloc(UDP_FRAME_SIZE * sizeof(char));
                if (req == NULL)
                    error_exit("Memory allocation failed!\n");
                bzero(req, UDP_FRAME_SIZE * sizeof(char));

                from = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
                if (from == NULL)
                    error_exit("Memory allocation failed!\n");
                bzero(from, sizeof(struct sockaddr));

                fromlen = (socklen_t *) malloc(sizeof(socklen_t));
                if (fromlen == NULL)
                    error_exit("Memory allocation failed!\n");
                bzero(fromlen, sizeof(socklen_t));
                *fromlen = sizeof(struct sockaddr_in); /* must be initialized again */

                /* receive incoming data */
                rb = recvfrom(ussd, /* udp socket descriptor */
                              &req[0], UDP_FRAME_SIZE-1, /* frame received and its size */
                              0, (struct sockaddr *)from, fromlen);

                if (g_verbose > 1)
                    fprintf(stdout, "MAIN: %s:%d --> recvfrom %d bytes\n",
                        inet_ntoa(from->sin_addr),
                        ntohs(from->sin_port),
                        rb);

                struct queue_item * item = (struct queue_item *) malloc(sizeof(struct queue_item));
                if (item == NULL)
                    error_exit("Memory allocation failed!\n");

                queue_item_new(item, req, rb, from, fromlen);

                /* Enqueue the new item in the proper queue */
                queue_head_add(&queues[sel_queue], item);

                /* Easier lock-free queue balancing */
                sel_queue = (sel_queue + 1) % g_nr_working_threads;

                if (g_verbose > 1) {
                    for (i = 0; i < g_nr_working_threads; i++) {
                        fprintf(stdout, "MAIN: == Thread number %d queue dumped ==\n", i);
                        queue_debug(&queues[i]);
                    }
                }
            }
        }
    }

    /* Wait all the threads before continuing */
    for (i = 0; i < g_nr_working_threads; i++)
        pthread_join(consumers[i], NULL);

    free(queues);
    free(consumers);
}

