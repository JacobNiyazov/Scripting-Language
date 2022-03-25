/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "sfmm.h"

void init_heap();
int extend_heap();
int coalesce();
int get_correct_free_list(size_t size);
int get_correct_quick_list(size_t size);


void *sf_malloc(sf_size_t size) {
    init_heap();
    if(size <= 0){
        return NULL;
    }
    size_t sizeNeeded = size + 8;
    if(sizeNeeded < 32)
        sizeNeeded = 32;
    sizeNeeded = ((sizeNeeded - 1)|15)+1;

    // check quick lists
    for(int i = 0; i < NUM_QUICK_LISTS; i++){
        if(sf_quick_lists[i].length == 0)
            continue;
        sf_block *temp = sf_quick_lists[i].first;
        int blksize = ((temp->header) ^ MAGIC) & 0x00000000FFFFFFF0;
        if(blksize == sizeNeeded){
            temp->header = (((uint64_t)size << 32) + (temp->header ^ MAGIC) - IN_QUICK_LIST) ^ MAGIC;

            sf_quick_lists[i].first = temp->body.links.next;
            sf_quick_lists[i].length = sf_quick_lists[i].length -1;
            return temp->body.payload;
        }
    }
    int err = 0;
    while(err == 0){
        // check free lists
        int index = get_correct_free_list(sizeNeeded);
        for(int i = index; i < NUM_FREE_LISTS; i++){
            sf_block *head = &sf_free_list_heads[i];
            sf_block *temp = head->body.links.next;

            while(temp != head){
                int blksize = (temp->header ^ MAGIC) & 0x00000000FFFFFFF0;
                if(blksize >= sizeNeeded){

                    // no splinter
                    if(blksize - sizeNeeded >= 32){
                        int newBlkSize = blksize - sizeNeeded;
                        sf_block *nextBlk = (sf_block *)((char *)temp + blksize);
                        // nextBlk->header = ((nextBlk->header ^ MAGIC) | PREV_BLOCK_ALLOCATED) ^ MAGIC;

                        int isPrevAloc = (temp->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
                        sf_block *new_free_blk = (sf_block *)((char *)temp + sizeNeeded);
                        new_free_blk->header = (newBlkSize + PREV_BLOCK_ALLOCATED) ^ MAGIC;

                        int newidx = get_correct_free_list(newBlkSize);
                        temp->body.links.prev->body.links.next = temp->body.links.next;
                        temp->body.links.next->body.links.prev = temp->body.links.prev;

                        new_free_blk->body.links.prev = &sf_free_list_heads[newidx];
                        new_free_blk->body.links.next = sf_free_list_heads[newidx].body.links.next;

                        sf_free_list_heads[newidx].body.links.next->body.links.prev = new_free_blk;
                        sf_free_list_heads[newidx].body.links.next = new_free_blk;


                        //temp->body.links.next->prev_footer = (newBlkSize + isPrevAloc) ^ MAGIC;
                        temp->header = (((uint64_t)size << 32) + sizeNeeded + THIS_BLOCK_ALLOCATED + isPrevAloc) ^ MAGIC;

                        nextBlk->prev_footer = (newBlkSize + PREV_BLOCK_ALLOCATED) ^ MAGIC;

                        return temp->body.payload;

                    }
                    // splinter
                    else{
                        sf_block *nextBlk = (sf_block *)((char *)temp + blksize);
                        nextBlk->header = ((nextBlk->header ^ MAGIC) | PREV_BLOCK_ALLOCATED) ^ MAGIC;

                        temp->body.links.prev->body.links.next = temp->body.links.next;
                        temp->body.links.next->body.links.prev = temp->body.links.prev;

                        int isPrevAloc = (temp->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
                        temp->header = (((uint64_t)size << 32) + blksize + isPrevAloc + THIS_BLOCK_ALLOCATED) ^ MAGIC;
                        return temp->body.payload;
                    }
                }
                temp = temp->body.links.next;

            }
        }
        // GROW HEAP
        int extend = extend_heap();
        if(extend){
            sf_errno = ENOMEM;
            return NULL;
        }

        coalesce();
    }
    void *output = NULL;
    return output;
}

void sf_free(void *pp) {
    // check bad cases
    if(pp == NULL)
        abort();
    if((uintptr_t)pp % 16 != 0)
        abort();
    sf_block *blk = pp - 16;
    int header = (blk->header) ^ MAGIC;
    if((header & 0x00000000FFFFFFF0) < 32)
        abort();
    if((header & 0x00000000FFFFFFF0) % 16 != 0)
        abort();
    if((char *)pp + sizeof(sf_header) < (char *)sf_mem_start())
        abort();
    if((char *)pp > (char *)sf_mem_end())
        abort();
    if(((blk->header ^ MAGIC) & THIS_BLOCK_ALLOCATED) == 0)
        abort();
    if(((blk->header ^ MAGIC) & PREV_BLOCK_ALLOCATED) == 0){
        if(((blk->prev_footer ^ MAGIC) & THIS_BLOCK_ALLOCATED) != 0){
            abort();
        }
    }

    size_t size = header & 0x00000000FFFFFFF0;
    int ql = get_correct_quick_list(size);
    // quick list
    if(ql != -1){
        // flush
        if(sf_quick_lists[ql].length == QUICK_LIST_MAX){
            for(int i = 0; i < QUICK_LIST_MAX; i++){
                sf_block *curr = sf_quick_lists[ql].first;
                size_t currSize = (curr->header ^ MAGIC) & 0x00000000FFFFFFF0;
                int isPrevAlloc = (curr->header ^ MAGIC) & PREV_BLOCK_ALLOCATED;
                curr->header = (0x0 | currSize | isPrevAlloc) ^ MAGIC;

                sf_block *next = (sf_block *)((char *)curr + currSize);
                next->prev_footer = (0x0 | currSize | isPrevAlloc) ^ MAGIC;


                sf_quick_lists[ql].first = curr->body.links.next;
                int index = get_correct_free_list(currSize);
                sf_free_list_heads[index].body.links.next->body.links.prev = curr;
                curr->body.links.next = sf_free_list_heads[index].body.links.next;
                curr->body.links.prev = &sf_free_list_heads[index];
                sf_free_list_heads[index].body.links.next = curr;
            }
            coalesce();
            sf_quick_lists[ql].length = 0;
        }

        // insert
        blk->header = (header + IN_QUICK_LIST) ^ MAGIC;
        sf_quick_lists[ql].length += 1;

        blk->body.links.next = sf_quick_lists[ql].first;
        sf_quick_lists[ql].first = blk;

        sf_quick_lists[ql].length = sf_quick_lists[ql].length + 1;

    }
    // free list
    else{
        int index = get_correct_free_list(size);
        int isPrevAlloc = (header & PREV_BLOCK_ALLOCATED);
        // insert
        blk->header = (size + isPrevAlloc) ^ MAGIC;
        blk->body.links.next = sf_free_list_heads[index].body.links.next;
        blk->body.links.prev = &sf_free_list_heads[index];
        sf_free_list_heads[index].body.links.next->body.links.prev = blk;
        sf_free_list_heads[index].body.links.next = blk;
        sf_block *next = (sf_block *)((char *)blk + size);
        next->prev_footer = (size + isPrevAlloc) ^ MAGIC;
        size_t nextSize = (next->header ^ MAGIC) & 0x00000000FFFFFFF0;
        int nextAlloc = (next->header ^ MAGIC) & THIS_BLOCK_ALLOCATED;
        int nextInQck = (next->header ^ MAGIC) & IN_QUICK_LIST;
        long psize = ((next->header ^ MAGIC) & 0xFFFFFFFF00000000);
        next->header = (psize + nextSize + nextAlloc + nextInQck) ^ MAGIC;

        if(nextAlloc == 0){
            sf_block *nextNextBlock = (sf_block *)((char *)next + nextSize);
            nextNextBlock->prev_footer = (nextSize + nextAlloc + nextInQck) ^ MAGIC;
        }

        coalesce();
    }
}

void *sf_realloc(void *pp, sf_size_t rsize) {
    // check bad cases
    void *newpp = (void *)((char *)pp - 16);
    sf_block *blk = (sf_block *)newpp;
    if(pp == NULL)
        abort();
    if((uintptr_t)pp % 16 != 0)
        abort();

    size_t size = (blk->header ^ MAGIC) & 0x00000000FFFFFFF0;
    if(((blk->header ^ MAGIC) & 0x00000000FFFFFFF0) < 32)
        abort();
    if(((blk->header ^ MAGIC) & 0x00000000FFFFFFF0) % 16 != 0)
        abort();
    if((char *)pp + sizeof(sf_header) < (char *)sf_mem_start())
        abort();
    if((char *)pp > (char *)sf_mem_end())
        abort();
    if(((blk->header ^ MAGIC) & THIS_BLOCK_ALLOCATED) == 0)
        abort();
    if(((blk->header ^ MAGIC) & PREV_BLOCK_ALLOCATED) == 0){
        if(((blk->prev_footer ^ MAGIC) & THIS_BLOCK_ALLOCATED) != 0){
            abort();
        }
    }
    if(rsize == 0){
        sf_free(pp);
        return NULL;
    }

    size_t sizeNeeded = rsize + sizeof(sf_header);
    if(sizeNeeded < 32)
        sizeNeeded = 32;
    sizeNeeded = ((sizeNeeded - 1)|15)+1;

    // larger block
    if(rsize >= size){
        sf_block *newBlk = (sf_block *)((char *)sf_malloc(rsize) -16);
        if(newBlk == NULL)
            return NULL;
        long psize = ((blk->header ^ MAGIC) & 0xFFFFFFFF00000000) >> 32;
        memcpy(blk->body.payload, newBlk->body.payload, psize);
        sf_free(pp);
        return newBlk->body.payload;
    }
    else{
        // splinter
        if((size - sizeNeeded) < 32){
            size_t paySize = (uint64_t)rsize << 32;
            blk->header = (((blk->header ^ MAGIC) & 0x00000000FFFFFFFF) + paySize) ^ MAGIC;
            return blk->body.payload;
        }
        else{
            size_t paySize = (uint64_t)rsize << 32;

            blk->header = (paySize + sizeNeeded + ((blk->header ^ MAGIC) & 0xF)) ^ MAGIC;
            sf_block *newFree = (sf_block *)((char *)blk + sizeNeeded);
            newFree->header = ((size - sizeNeeded) + PREV_BLOCK_ALLOCATED) ^ MAGIC;
            int index = get_correct_free_list(size - sizeNeeded);
            newFree->body.links.next = sf_free_list_heads[index].body.links.next;
            newFree->body.links.prev = &sf_free_list_heads[index];
            sf_free_list_heads[index].body.links.next->body.links.prev = newFree;
            sf_free_list_heads[index].body.links.next = newFree;

            sf_block *afterFree = (sf_block *)((char *)newFree + (size - sizeNeeded));
            afterFree->prev_footer = ((size - sizeNeeded) + PREV_BLOCK_ALLOCATED) ^ MAGIC;
            coalesce();
            return blk->body.payload;
        }

    }
}

double sf_internal_fragmentation() {
    double payloadAMT = 0.0;
    double blockAMT = 0.0;

    sf_block *curr = (sf_block *)(sf_mem_start() + sizeof(sf_block));
    while(curr != (sf_mem_end()-16)){
        size_t currSize = (curr->header ^ MAGIC) & 0x00000000FFFFFFF0;
        blockAMT += currSize;
        long psize = ((curr->header ^ MAGIC) & 0xFFFFFFFF00000000) >> 32;
        payloadAMT += psize;

        curr = (sf_block *)((char *)curr + currSize);
    }

    if(blockAMT == 0.0)
        return 0.0;
    else
        return (payloadAMT/blockAMT);

}

double sf_peak_utilization() {
    double payloadAMT = 0.0;

    sf_block *curr = (sf_block *)(sf_mem_start() + sizeof(sf_block));
    while(curr != (sf_mem_end()-16)){
        size_t currSize = (curr->header ^ MAGIC) & 0x00000000FFFFFFF0;
        long psize = ((curr->header ^ MAGIC) & 0xFFFFFFFF00000000) >> 32;
        payloadAMT += psize;

        curr = (sf_block *)((char *)curr + currSize);
    }


    if((sf_mem_end() - sf_mem_start()) == 0)
        return 0.0;
    else
        return (payloadAMT/(sf_mem_end() - sf_mem_start()));
}

void init_heap(){
    if(sf_mem_start() == sf_mem_end()){
        char *heap = sf_mem_grow();
        if(heap == NULL){
            //error
        }

        for(int i = 0; i < NUM_FREE_LISTS; i++){
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];

        }
        sf_block prologue = { .header = (0x0 | sizeof(sf_block) | THIS_BLOCK_ALLOCATED) ^ MAGIC };
        *(sf_block *)(sf_mem_start()) = prologue;

        sf_block fblock = { .header = ((PAGE_SZ - (sizeof(sf_header) + sizeof(sf_block)) - 8)|PREV_BLOCK_ALLOCATED) ^ MAGIC };
        *(sf_block *)(sf_mem_start() + sizeof(sf_block)) = fblock;

        // sf_footer free_footer = 0x00000000000003D0 ^ MAGIC;
        // *(sf_footer *)(sf_mem_start() + 1008) = free_footer;


        sf_block *freeblk = (sf_mem_start() + sizeof(sf_block));
        sf_free_list_heads[5].body.links.next = freeblk;
        sf_free_list_heads[5].body.links.prev = freeblk;
        // sf_free_list_heads[5].prev_footer = 0x00000000000003D1 ^ MAGIC;
        freeblk->body.links.prev = &sf_free_list_heads[5];
        freeblk->body.links.next = &sf_free_list_heads[5];

        sf_block epilogue = { .prev_footer =  ((PAGE_SZ - (sizeof(sf_header) + sizeof(sf_block)) - 8)|PREV_BLOCK_ALLOCATED) ^ MAGIC, .header = THIS_BLOCK_ALLOCATED ^ MAGIC };
        *(sf_block *)(sf_mem_end() - (sizeof(sf_header)+ sizeof(sf_header))) = epilogue;
        // sf_block *ep = sf_mem_end() - 8;
        // ep->prev_footer = 0x00000000000003D4 ^ MAGIC;
    }

}

int get_correct_free_list(size_t size){
    if(size <= 32)
        return 0;
    else if((size > 32) & (size <= 64))
        return 1;
    else if((size > 64) & (size <= 128))
        return 2;
    else if((size > 128) & (size <= 256))
        return 3;
    else if((size > 256) & (size <= 512))
        return 4;
    else if((size > 512) & (size <= 1024))
        return 5;
    else if((size > 1024) & (size <= 2048))
        return 6;
    else if((size > 2048) & (size <= 4096))
        return 7;
    else if((size > 4096) & (size <= 8192))
        return 8;
    else
        return 9;

}

int get_correct_quick_list(size_t size){
    if(size == 32)
        return 0;
    else if(size == 48)
        return 1;
    else if(size == 64)
        return 2;
    else if(size == 80)
        return 3;
    else if(size == 96)
        return 4;
    else if(size == 112)
        return 5;
    else if(size == 128)
        return 6;
    else if(size == 144)
        return 7;
    else if(size == 160)
        return 8;
    else if(size == 176)
        return 9;
    else
        return -1;

}

int extend_heap(){
    char *heap = sf_mem_grow();
    if(heap == NULL){
        return -1;
    }
    sf_block *oldepi = (sf_block *)(heap - 16);
    int isPrevAlloc = ((oldepi -> header) ^ MAGIC) & PREV_BLOCK_ALLOCATED;
    oldepi->header  = (0x400 | isPrevAlloc) ^ MAGIC;

    int ind = get_correct_free_list(1024);
    oldepi->body.links.next = sf_free_list_heads[ind].body.links.next;
    oldepi->body.links.prev = &sf_free_list_heads[ind];

    sf_free_list_heads[ind].body.links.next->body.links.prev = oldepi;
    sf_free_list_heads[ind].body.links.next = oldepi;

    // sf_block *newblk = (sf_block *)heap;
    // newblk -> header = (0x00000000000003F0) ^ MAGIC;
    // newblk -> prev_footer = (0x18 | isPrevAlloc) ^ MAGIC;
    // size_t nextSize = (newblk->header ^ MAGIC) & 0x00000000FFFFFFF0;

    // int nextInd = get_correct_free_list(nextSize);
    // newblk->body.links.next = sf_free_list_heads[nextInd].body.links.next;
    // newblk->body.links.prev = &sf_free_list_heads[nextInd];
    // sf_free_list_heads[nextInd].body.links.next->body.links.prev = newblk;
    // sf_free_list_heads[nextInd].body.links.next = newblk;

    sf_block epilogue = { .prev_footer =  (0x400 | isPrevAlloc) ^ MAGIC, .header = 0x0000000000000004 ^ MAGIC };
    *(sf_block *)(sf_mem_end() - 16) = epilogue;
    return 0;
}

int coalesce(){
    char *pos = sf_mem_start() + 32;
    while(pos != sf_mem_end()-16){
        sf_block *currBlk = (sf_block *)pos;
        int currAlloc = ((currBlk -> header) ^ MAGIC) & THIS_BLOCK_ALLOCATED;
        size_t size = ((currBlk -> header) ^ MAGIC) & 0x00000000FFFFFFF0;
        if(currAlloc == 0){
            sf_block *nextblk = (sf_block *)(pos + size);
            int nextAlloc = ((nextblk -> header) ^ MAGIC) & THIS_BLOCK_ALLOCATED;
            if(nextAlloc == 0){
                size_t nextSize = ((nextblk -> header) ^ MAGIC) & 0x00000000FFFFFFF0;
                currBlk -> header = (((currBlk -> header) ^ MAGIC) + nextSize) ^ MAGIC;
                sf_block *nextNextBlock = (sf_block *)(pos + size + nextSize);
                nextNextBlock -> prev_footer = currBlk -> header;

                size_t newSize = ((currBlk -> header) ^ MAGIC) & 0x00000000FFFFFFF0;
                // int index = get_correct_free_list(size);
                // int nextBlkIndex = get_correct_free_list(nextSize);
                int newIndex = get_correct_free_list(newSize);

                //remove blocks
                currBlk->body.links.prev->body.links.next = currBlk->body.links.next;
                currBlk->body.links.next->body.links.prev = currBlk->body.links.prev;

                nextblk->body.links.prev->body.links.next = nextblk->body.links.next;
                nextblk->body.links.next->body.links.prev = nextblk->body.links.prev;

                // insert new block
                currBlk->body.links.next = sf_free_list_heads[newIndex].body.links.next;
                currBlk->body.links.prev = &sf_free_list_heads[newIndex];
                sf_free_list_heads[newIndex].body.links.next->body.links.prev = currBlk;
                sf_free_list_heads[newIndex].body.links.next = currBlk;

                // if((index != newIndex) & ext){
                //     currBlk->body.links.prev->body.links.next = currBlk->body.links.next;
                //     currBlk->body.links.next->body.links.prev = currBlk->body.links.prev;

                //     currBlk->body.links.prev = &sf_free_list_heads[newIndex];
                //     currBlk->body.links.next = sf_free_list_heads[newIndex].body.links.next;
 
                //     sf_free_list_heads[newIndex].body.links.next->body.links.prev = currBlk;
                //     sf_free_list_heads[newIndex].body.links.next = currBlk;
                // }
                // if((index != newIndex) & (ext == 0)){
                //     currBlk->body.links.prev->body.links.next = currBlk->body.links.next->body.links.next;
                //     currBlk->body.links.next->body.links.next->body.links.prev = currBlk->body.links.prev;

                //     currBlk->body.links.prev = &sf_free_list_heads[newIndex];
                //     currBlk->body.links.next = sf_free_list_heads[newIndex].body.links.next;
 
                //     sf_free_list_heads[newIndex].body.links.next->body.links.prev = currBlk;
                //     sf_free_list_heads[newIndex].body.links.next = currBlk;
                // }
                // else{
                //     currBlk->body.links.next = currBlk->body.links.next->body.links.next;
                //     currBlk->body.links.next->body.links.prev = currBlk;
                // }
                continue;
            }
        }
        pos = pos + size;
    }
    return 0;
}