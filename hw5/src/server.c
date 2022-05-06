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
    if(t_u == NULL){
        close(connectionfd);
        return NULL;
    }
    int reg = pbx_register(pbx, t_u, connectionfd);
    if(reg == -1){
        close(connectionfd);
        return NULL;
    }

    char *ch = malloc(1);
    if(ch == NULL){
        pbx_unregister(pbx, t_u);
        close(connectionfd);
        return NULL;
    }
    char *ch_two = malloc(1);
    if(ch_two == NULL){
        free(ch);
        pbx_unregister(pbx, t_u);
        close(connectionfd);
        return NULL;
    }
    FILE *stream;
    char *buf;
    while(1){
        size_t len = 0;
        // debug("outer\n");
        stream = open_memstream(&buf, &len);
        if(stream == NULL){
            pbx_unregister(pbx, t_u);
            close(connectionfd);
            return NULL;
        }
        int r = rio_readn(connectionfd, ch, 1);
        // debug("r = %d\n", r);
        while(r > 0){
            // debug("inner\n");
            if(strcmp(ch, "\r") == 0){
                // *ch_two = NULL;
                rio_readn(connectionfd, ch_two, 1);
                if(strcmp(ch_two, "\n") == 0){
                    // free(ch_two);
                    break;
                }
                else{
                    if(fprintf(stream, "%s", ch) == -1){
                        free(ch);
                        free(ch_two);
                        free(buf);
                        fclose(stream);
                        pbx_unregister(pbx, t_u);
                        close(connectionfd);
                        return NULL;
                    }
                    if(fflush(stream) < 0){
                        free(ch);
                        free(ch_two);
                        free(buf);
                        fclose(stream);
                        pbx_unregister(pbx, t_u);
                        close(connectionfd);
                        return NULL;
                    }
                    if(fprintf(stream, "%s", ch_two) == -1){
                        free(ch);
                        free(ch_two);
                        free(buf);
                        fclose(stream);
                        pbx_unregister(pbx, t_u);
                        close(connectionfd);
                        return NULL;
                    }
                    if(fflush(stream) < 0){
                        free(ch);
                        free(ch_two);
                        free(buf);
                        fclose(stream);
                        pbx_unregister(pbx, t_u);
                        close(connectionfd);
                        return NULL;
                    }
                    // free(ch_two);
                }
            }
            else{
                if(fprintf(stream, "%s", ch) == -1){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    fclose(stream);
                    pbx_unregister(pbx, t_u);
                    close(connectionfd);
                    return NULL;
                }
                if(fflush(stream) < 0){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    fclose(stream);
                    pbx_unregister(pbx, t_u);
                    close(connectionfd);
                    return NULL;
                }
                r = rio_readn(connectionfd, ch, 1);
            }
        }
        // free(ch);
        fclose(stream);

        if(r <= 0){
            free(ch);
            free(ch_two);
            free(buf);
            pbx_unregister(pbx, t_u);
            close(connectionfd);
            return NULL;
        }

        if(strcmp(buf, tu_command_names[TU_PICKUP_CMD]) == 0){
            if(tu_pickup(t_u) == -1){
                free(ch);
                free(ch_two);
                free(buf);
                pbx_unregister(pbx, t_u);
                close(connectionfd);
                return NULL;
            }
        }
        else if(strcmp(buf, tu_command_names[TU_HANGUP_CMD]) == 0){
            if(tu_hangup(t_u) == -1){
                free(ch);
                free(ch_two);
                free(buf);
                pbx_unregister(pbx, t_u);
                close(connectionfd);
                return NULL;
            }
        }
        else if(strncmp(buf, "dial ", 5) == 0){
            char **end = NULL;
            long res = strtol(buf+5, end, 10);
            if(res){
                if(pbx_dial(pbx, t_u, res) == -1){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    pbx_unregister(pbx, t_u);
                    close(connectionfd);
                    return NULL;
                }
            }
            else{
                if(errno != 0){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    pbx_unregister(pbx, t_u);
                    close(connectionfd);
                    return NULL;
                }
                else if((buf+5) == *end){
                    free(ch);
                    free(ch_two);
                    free(buf);
                    pbx_unregister(pbx, t_u);
                    close(connectionfd);
                    return NULL;
                }
                else{
                    if(pbx_dial(pbx, t_u, 0) == -1){
                        free(ch);
                        free(ch_two);
                        free(buf);
                        pbx_unregister(pbx, t_u);
                        close(connectionfd);
                        return NULL;
                    }
                }

            }
        }
        else if(strncmp(buf, "chat ", 5) == 0){
            if(tu_chat(t_u, buf+5) == -1){
                free(ch);
                free(ch_two);
                free(buf);
                pbx_unregister(pbx, t_u);
                close(connectionfd);
                return NULL;
            }
        }
        else if(strncmp(buf, tu_command_names[TU_CHAT_CMD], 4) == 0){
            if(tu_chat(t_u, buf+4) == -1){
                free(ch);
                free(ch_two);
                free(buf);
                pbx_unregister(pbx, t_u);
                close(connectionfd);
                return NULL;
            }
        }

        free(buf);

    }
    free(ch);
    free(ch_two);
    pbx_unregister(pbx, t_u);
    close(connectionfd);
    return NULL;

}
// #endif
