/*
 * PBX: simulates a Private Branch Exchange.
 */
#include <stdlib.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/socket.h>

#include "pbx.h"
#include "debug.h"


typedef struct pbx{
    TU *tu_array[FD_SETSIZE];
}PBX;

sem_t pbx_sem;

/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
// #if 0
PBX *pbx_init() {
    PBX *p = calloc(FD_SETSIZE, sizeof(TU*));
    if(p == NULL)
        return NULL;
    sem_init(&pbx_sem, 0, 1);
    return p;
}
// #endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
// #if 0
void pbx_shutdown(PBX *pbx) {
    sem_wait(&pbx_sem);
    // debug("pbx_shutdown\n");
    for(int i = 0; i < FD_SETSIZE; i++){
        if(pbx->tu_array[i] != NULL){
            int res = shutdown(tu_fileno(pbx->tu_array[i]), SHUT_RDWR);
            if(res == EBADF || res == EINVAL || res == ENOTCONN || res == ENOTSOCK){
                free(pbx);
                exit(1);
            }
        }
    }
    for(int i = 0; i < FD_SETSIZE; i++){
        if(pbx->tu_array[i] != NULL){
            pbx->tu_array[i] = NULL;
        }
    }
    sem_post(&pbx_sem);
    sem_destroy(&pbx_sem);
    free(pbx);
}
// #endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
// #if 0
int pbx_register(PBX *pbx, TU *tu, int ext) {
    sem_wait(&pbx_sem);
    int set = 0;
    for(int i = 0; i < FD_SETSIZE; i++){
        if(pbx->tu_array[i] == NULL){
            pbx->tu_array[i] = tu;
            set = 1;
            break;
        }
    }
    sem_post(&pbx_sem);
    if(!set)
        return -1;
    int se = tu_set_extension(tu, ext);
    if(se == -1)
        return -1;
    tu_ref(tu, "pbx_register");

    return 0;
}
// #endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
// #if 0
int pbx_unregister(PBX *pbx, TU *tu) {
    int th = tu_hangup(tu);
    if(th == -1)
        return -1;
    
    sem_wait(&pbx_sem);
    for(int i = 0; i < FD_SETSIZE; i++){
        if(pbx->tu_array[i] == tu){
            pbx->tu_array[i] = NULL;
            // debug("middle\n");
            break;
        }
    }
    sem_post(&pbx_sem);
    // debug("almost\n");
    tu_unref(tu, "pbx_unregister");
    // debug("done\n");
    return 0;
}
// #endif

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
// #if 0
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    TU *other = NULL;
    for(int i = 0; i < FD_SETSIZE; i++){
        if(tu_extension(pbx->tu_array[i]) == ext){
            other = pbx->tu_array[i];
            break;
        }
    }
    if(other){
        int td = tu_dial(tu, other);
        if(td == -1)
            return -1;
    }
    else
        return -1;
    return 0;
}
// #endif
