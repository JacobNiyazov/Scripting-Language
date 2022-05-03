/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h"

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
// #if 0
void *pbx_client_service(void *arg) {
    int connectionfd = *((int *)arg);
    Free(arg);
    Pthread_detach(pthread_self());

    TU *t_u = tu_init(connectionfd);
    pbx_register(pbx, t_u, connectionfd);

    char *ch = malloc(1);
    if(ch == NULL){
        exit(1);
    }
    char *ch_two = malloc(1);
    if(ch_two == NULL){
        exit(1);
    }
    FILE *stream;
    char *buf;
    while(1){
        size_t len = 0;

        stream = open_memstream(&buf, &len);
        rio_readn(connectionfd, ch, 1);
        while(ch){
            if(strcmp(ch, "\r") == 0){
                // *ch_two = NULL;
                rio_readn(connectionfd, ch_two, 1);
                if(strcmp(ch_two, "\n") == 0){
                    // free(ch_two);
                    break;
                }
                else{
                    fprintf(stream, "%s", ch);
                    fflush(stream);
                    fprintf(stream, "%s", ch_two);
                    fflush(stream);
                    // free(ch_two);
                }
            }
            else{
                fprintf(stream, "%s", ch);
                fflush(stream);
                rio_readn(connectionfd, ch, 1);
            }
        }
        // free(ch);
        fclose(stream);
        if(strcmp(buf, "pickup") == 0){
            tu_pickup(t_u);
        }
        else if(strcmp(buf, "hangup") == 0){
            tu_hangup(t_u);
        }
        else if(strncmp(buf, "dial ", 5) == 0){
            char **end = NULL;
            long res = strtol(buf+5, end, 10);
            if(res){
                pbx_dial(pbx, t_u, res);
            }
            else{
                if(errno != 0){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    exit(1);
                }
                else if((buf+5) == *end){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    exit(1);
                }
                else{
                    pbx_dial(pbx, t_u, 0);
                }

            }
        }
        else if(strncmp(buf, "chat ", 5) == 0){
            tu_chat(t_u, buf+5);
        }
        else if(strncmp(buf, "chat", 4) == 0){
            tu_chat(t_u, buf+4);
        }

        free(buf);

    }
    free(ch);
    free(ch_two);


}
// #endif
