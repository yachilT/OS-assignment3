#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROC 4
#define PG_SIZE 4096

int min(int a, int b) {
    return a > b ? b : a;
}

int main(int argc, char *argv[]) {
    void* buffer = malloc(PG_SIZE);

    // reset buffer to zero
    memset(buffer, 0, PG_SIZE);

    int parent_pid = getpid();
    uint16 id = 0;

    for (uint16 i = 0; i < NUM_PROC; i++) {
        if (fork() == 0) {
            id = i+1;
            break;
        }
    }

    if (id != 0) { // is child
        void* base_addr = map_shared_pages(parent_pid, buffer, PG_SIZE);
        void* va = base_addr;

        while (va + 4 <= base_addr + PG_SIZE) {
            char msg[id + 1];
            for (int i = 0; i < id; i++) { msg[i] = 'a'; }
            msg[id] = 0;
            uint32 length = min(strlen(msg), base_addr + PG_SIZE - va - 4);
            uint32 header = (((uint32)id) << 16) | length;
            if (__sync_val_compare_and_swap((int*)va, 0, header) == 0) { // if swap succeeded
                va += 4;
                memmove(va, msg, length);
                sleep(1); // to avoid first proccess winning every race condition
            } else {
                length = (uint16) (*(int*)va);
                va+= 4;
            }

            va += length;
            va = (void*)(((uint64)va + 3) & ~3);
        }
    }
    else {
        void * va = buffer;
        while (va < buffer + PG_SIZE) {
            int header = *(int*)va;
            if (header) {
                uint16 length = (uint16)header;
                uint16 index = (uint16)(header >> 16);
                char msg[length + 1];

                va+= 4;
                memmove(msg, va, length);
                msg[length] = 0;
                printf("Parent: child %d printed %s\n", index, msg);

                va += length;
                va = (void*)(((uint64)va + 3) & ~3);
            }
        }
        while (wait(0) != -1);
    }

    return 0;
}