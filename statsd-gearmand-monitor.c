#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <syslog.h>
#include "lib/statsd-c-client/statsd-client.c"
#include "statsd-gearmand-monitor.h"


static int running = 1;

void sigterm(int sig)
{
    running = 0;
}




void showHelp() {
    printf("statsd-gearmand-monitor version %s\n", VERSION);
    printf("sends stats about gearmand jobs & workers to statsd\n\n");
    printf("Usage: statsd-gearmand-monitor [OPTIONS]\n");
    printf("\t-?                  this help\n");
    printf("\t-N <metric>         metric prefix to update (required)\n");
    printf("\t-h <host>           statsd host (default: %s)\n", STATSD_DEFAULT_HOST);
    printf("\t-p <port>           statsd port (default: %d)\n", STATSD_DEFAULT_PORT);
    printf("\t-s <rate>           sample rate (default: %2.2f)\n", DEFAULT_SAMPLE_RATE);
    printf("\t-H <host>           gearman host (default: %s)\n", GEARMAND_DEFAULT_HOST);
    printf("\t-P <port>           gearman port (default: %d)\n", GEARMAND_DEFAULT_PORT);
    printf("\t-t <time>           polling rate (seconds) (default: %d)\n", DEFAULT_POLLING_INTERVAL);
    printf("\t-f                  run in foreground mode\n");
    printf("\t-d                  debug mode\n");

}


char * _metric_name(char *function, char *metric) {

    char *metric_name = NULL;

    size_t fnlen = strlen(function);
    size_t mlen = strlen(metric);

    metric_name = malloc(fnlen + mlen + 2);


    strcpy(metric_name, function);
    strcat(metric_name, ".");
    strcat(metric_name, metric);

    metric_name[fnlen + mlen + 1] = 0;

    return metric_name;

}

int main(int argc, char *argv[]) {


    char *metric_name = NULL;
    int c;

    float sample_rate = DEFAULT_SAMPLE_RATE;
    char *statsd_host = STATSD_DEFAULT_HOST;
    int statsd_port = STATSD_DEFAULT_PORT;

    char *gearmand_host = GEARMAND_DEFAULT_HOST;
    int gearmand_port = GEARMAND_DEFAULT_PORT;

    int debug = 0;
    int foreground = 0;
    int polling_interval = DEFAULT_POLLING_INTERVAL;


while ((c = getopt (argc, argv, "e:dvifH::t::P::h::p::s:N:")) != -1) {
        switch (c) {
            case '?':
                showHelp();
                return 0;
            break;
            case 'N':
                metric_name = malloc(strlen(optarg));
                strcpy(metric_name, optarg);
            break;
            case 'h': 
                statsd_host = optarg;
            break;
            case 'p':
                statsd_port = atoi(optarg);
            break;
            case 's':
                sample_rate = (float) atof(optarg);
            break;
            case 'H':
                gearmand_host = optarg;
            break;
            case 'P':
                gearmand_port = atoi(optarg);
            break;
            case 't':
                polling_interval = atoi(optarg);
            break;            
            case 'f':
                foreground = 1;
            break;       
            case 'd':
                debug = 1;
            break;
            default:
                showHelp();
                return 1;
            break;
        }
    }

    if (metric_name == NULL) {
        fprintf(stderr, "%s: missing -N option\n", argv[0]);
        showHelp();
        return 1;
    }

    if (debug) {
        setlogmask(LOG_UPTO(LOG_INFO));
        openlog("statsd-gearmand-monitor",  LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
        syslog(LOG_INFO, "statsd host: %s:%d", statsd_host, statsd_port);
        syslog(LOG_INFO, "metric name: %s", metric_name);
        syslog(LOG_INFO, "sample rate: %2.2f", sample_rate);
        syslog(LOG_INFO, "polling interval: %d secs", polling_interval);
        syslog(LOG_INFO, "gearmand host: %s:%d", gearmand_host, gearmand_port);
    }


    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTERM, sigterm);

    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in gearmand_serv_addr; 


    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&gearmand_serv_addr, '0', sizeof(gearmand_serv_addr)); 

    gearmand_serv_addr.sin_family = AF_INET;
    gearmand_serv_addr.sin_port = htons(gearmand_port); 

    if(inet_pton(AF_INET, gearmand_host, &gearmand_serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    //daemon(0, 0);

    statsd_link *_statsd_link;

    _statsd_link = statsd_init_with_namespace(statsd_host, statsd_port, metric_name);


    while(running) {

        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Error : Could not create socket \n");
            return 1;
        } 

        if( connect(sockfd, (struct sockaddr *)&gearmand_serv_addr, sizeof(gearmand_serv_addr)) < 0)
        {
            printf("\n Error : Connect Failed to gearmand \n");
            return 1;
        }

        write(sockfd, "status\n", strlen("status\n")); 

        while ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
        {
            recvBuff[n] = 0;

            if (recvBuff[n - 2] == '.' && recvBuff[n - 1] == '\n') {
                recvBuff[n - 2] = 0;
                break;
            }
        }

        close(sockfd);

        char * pch;
        pch = strtok (recvBuff, "\n");
        while (pch != NULL)
        {
            char function_name[1024];
            int queued, running, connected;
            sscanf(pch, "%s\t%d\t%d\t%d", function_name, &queued, &running, &connected);

            statsd_gauge(_statsd_link, _metric_name(function_name, "connected"), connected);
            statsd_gauge(_statsd_link, _metric_name(function_name, "queued") , queued);
            statsd_gauge(_statsd_link, _metric_name(function_name, "running") , running);


            if (debug) {
                 syslog(LOG_INFO, "function %s has a total of %d jobs queued %d running - %d workers connected\n", function_name, queued, running, connected);
            }

            pch = strtok (NULL, "\n");
        }

        sleep(polling_interval);
    }

    statsd_finalize(_statsd_link);

    return 0;
}