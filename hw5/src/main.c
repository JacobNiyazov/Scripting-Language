#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>


#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

static void terminate(int status);

void handle_sighup(){
    terminate(0);
}

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    if(argc != 3)
        terminate(EXIT_FAILURE);

    if(strcmp(argv[1], "-p") != 0)
        terminate(EXIT_FAILURE);

    int port = atoi(argv[2]);
    if(port < 1024)
        terminate(EXIT_FAILURE);

    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // fprintf(stderr, "You have to finish implementing main() "
	   //  "before the PBX server will function.\n");

    // signal handler
    struct sigaction sa;
    sa.sa_handler = &handle_sighup;
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGHUP, &sa, NULL) == -1)
        terminate(EXIT_FAILURE);

    int listen, *connection;
    pthread_t threadID;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(struct sockaddr_in);

    listen = Open_listenfd_t(argv[2]);
    while(1){
        connection = Malloc(sizeof(int));
        *connection = Accept(listen, (SA *) &clientaddr, &clientlen);
        Pthread_create(&threadID, NULL, pbx_client_service, connection);
    }

    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}


