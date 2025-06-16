#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    int disable_unmap = 0;
    if (argc > 1 && strcmp(argv[1], "-n") == 0)
        disable_unmap = 1;
    
    char* s = "Hello Daddy\n";
    int parent_pid = getpid();
    void* paddr = malloc(strlen(s)); // virtual address of parent

    int pid = fork();
    if (pid == 0) {
        printf("Child: before mapping: %p\n", sbrk(0));
        
        void* caddr = map_shared_pages(parent_pid, paddr, strlen(s)); // virtual address of child. it is mapped to the same phy address as paddr

        printf("Child: after mapping: %p\n", sbrk(0));
        strcpy(caddr, s);

        if (!disable_unmap) {
            if (unmap_shared_pages(caddr, strlen(s)) == -1)
                printf("Child: failed to unmap\n");
            printf("Child: after unmapping: %p\n", sbrk(0));
            printf("Child: malloc address %p\n", malloc(64));
            printf("Child: after malloc: %p\n", sbrk(0));
        }
        exit(0);
    } else {
        wait(0);
        printf("Parent: %s", paddr);
        exit(0);
    }
}
