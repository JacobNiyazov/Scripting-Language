/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"

typedef struct tu {
    int fd;
    int extension;
    int refCount;
    struct tu *peer;
    TU_STATE current_state;
} TU;

/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 */
// #if 0
TU *tu_init(int fd) {
    TU *out = calloc(1, sizeof(TU)); //*TU or TU
    if(out == NULL)
        exit(1);
    out->fd = fd;
    out->extension = fd;
    out->current_state = TU_ON_HOOK;
    return out;
}
// #endif

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 */
// #if 0
void tu_ref(TU *tu, char *reason) {
    tu->refCount = tu->refCount + 1;
}
// #endif

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 */
// #if 0
void tu_unref(TU *tu, char *reason) {
    tu->refCount = tu->refCount - 1;
}
// #endif

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
// #if 0
int tu_fileno(TU *tu) {
    if(tu->fd)
        return tu->fd;
    else
        return -1;
}
// #endif

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */
// #if 0
int tu_extension(TU *tu) {
    if(tu->extension)
        return tu->extension;
    else
        return -1;
}
// #endif

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 */
// #if 0
int tu_set_extension(TU *tu, int ext) {
    tu->extension = ext;
    char str[12];
    if(sprintf(str, "ON HOOK %d\r\n", ext) == -1)
        return -1;
    if(write(ext, str, sizeof(str)) == -1)
        return -1;
    return 0;
}
// #endif

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_dial(TU *tu, TU *target) {
    if(target == NULL){
        if(tu->current_state == TU_DIAL_TONE){
            tu->current_state = TU_ERROR;
            char str[8];
            if(sprintf(str, "ERROR\r\n") == -1)
                return -1;
            if(write(tu->fd, str, sizeof(str)) == -1)
                return -1;
        }
        return 0;

    }
    if(tu->current_state != TU_DIAL_TONE)
        return 0;
    if(tu == target){
        tu->current_state = TU_BUSY_SIGNAL;
        char str[14];
        if(sprintf(str, "BUSY SIGNAL\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;
        return 0;
    }
    if((target->peer != NULL) || (target->current_state != TU_ON_HOOK)){
        tu->current_state = TU_BUSY_SIGNAL;
        char str[14];
        if(sprintf(str, "BUSY SIGNAL\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;
        return 0;
    }

    tu->peer = target;
    target->peer = tu;

    tu->refCount = tu->refCount + 1;
    target->refCount = target->refCount + 1;

    target->current_state = TU_RINGING;
    char str[10];
    if(sprintf(str, "RINGING\r\n") == -1)
        return -1;
    if(write(target->fd, str, sizeof(str)) == -1)
        return -1;

    tu->current_state = TU_RING_BACK;
    char strt[12];
    if(sprintf(strt, "RING BACK\r\n") == -1)
        return -1;
    if(write(tu->fd, strt, sizeof(strt)) == -1)
        return -1;

    return 0;
}
// #endif

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_pickup(TU *tu) {
    if(tu->current_state != TU_ON_HOOK && tu->current_state != TU_RINGING){
        return 0;
    }
    if(tu->current_state == TU_ON_HOOK){
        tu->current_state = TU_DIAL_TONE;
        char str[12];
        if(sprintf(str, "DIAL TONE\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;
        return 0;
    }
    else if(tu->current_state == TU_RINGING){
        tu->current_state = TU_CONNECTED;
        char str[14];
        if(sprintf(str, "CONNECTED %d\r\n", tu->peer->extension) == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;

        tu->peer->current_state = TU_CONNECTED;
        char strt[14];
        if(sprintf(strt, "CONNECTED %d\r\n", tu->extension) == -1)
            return -1;
        if(write(tu->peer->fd, strt, sizeof(strt)) == -1)
            return -1;
        return 0;
    }
    return 0;
}
// #endif

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_hangup(TU *tu) {
    if(tu->current_state == TU_CONNECTED || tu->current_state == TU_RINGING){
        tu->current_state = TU_ON_HOOK;
        char str[12];
        if(sprintf(str, "ON HOOK\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;

        tu->peer->current_state = TU_DIAL_TONE;
        char strt[12];
        if(sprintf(strt, "DIAL TONE\r\n") == -1)
            return -1;
        if(write(tu->peer->fd, strt, sizeof(strt)) == -1)
            return -1;

        return 0;
    }
    if(tu->current_state == TU_RING_BACK){
        tu->current_state = TU_ON_HOOK;
        char str[12];
        if(sprintf(str, "ON HOOK\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;

        tu->peer->current_state = TU_ON_HOOK;
        char strt[12];
        if(sprintf(strt, "ON HOOK\r\n") == -1)
            return -1;
        if(write(tu->peer->fd, strt, sizeof(strt)) == -1)
            return -1;

        return 0;
    }
    if(tu->current_state == TU_DIAL_TONE || tu->current_state == TU_BUSY_SIGNAL || tu->current_state == TU_ERROR){
        tu->current_state = TU_ON_HOOK;
        char str[12];
        if(sprintf(str, "ON HOOK\r\n") == -1)
            return -1;
        if(write(tu->fd, str, sizeof(str)) == -1)
            return -1;
        return 0;
    }
    return 0;
}
// #endif

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */
// #if 0
int tu_chat(TU *tu, char *msg) {
    if(tu->current_state != TU_CONNECTED){
        return -1;
    }

    if(write(tu->peer->fd, msg, sizeof(msg)) == -1)
        return -1;

    char str[14];
    if(sprintf(str, "CONNECTED %d\r\n", tu->peer->extension) == -1)
        return -1;
    if(write(tu->fd, str, sizeof(str)) == -1)
        return -1;

    return 0;
}
// #endif
