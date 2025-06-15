#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int main(int argc, char *argv[]) {
    int disable_unmap = 0;
    if (argc > 1 && strcmp(argv[1], "-n") == 0)
        disable_unmap = 1;

    printf("Child starting...\n");
    void* before = sbrk(0);
    printf("Before mapping: sz = %p\n", before);

    int p[2];
    pipe(p);

    int pid = fork();
    if (pid < 0) {
        printf("Fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // child
        char *shared = (char *)sbrk(4096); // allocate a page
        strcpy(shared, "Hello daddy");
        void* after_mapping = sbrk(0);
        printf("Child: After mapping: sz = %p\n", after_mapping);

        // Tell parent the address
        write(p[1], &shared, sizeof(shared));

        if (!disable_unmap) {
            unmap_shared_pages(shared, 4096);  // optional unmap syscall
            void* after_unmap = sbrk(0);
            printf("Child: After unmap: sz = %p\n", after_unmap);
        }

        char *p2 = malloc(64);
        printf("Child: malloc after unmap: %p\n", p2);

        exit(0);
    } else {
        wait(0);
        void *child_va;
        read(p[0], &child_va, sizeof(child_va));

        void *mapped = map_shared_pages(pid, child_va, 4096);
        printf("Parent: received mapping at %p\n", mapped);
        printf("Parent sees: %s\n", (char *)mapped);

        void* after = sbrk(0);
        printf("Parent: sz after mapping: %p\n", after);

        if (!disable_unmap) {
            unmap_shared_pages(mapped, 4096);
            void* after_unmap = sbrk(0);
            printf("Parent: After unmap: sz = %p\n", after_unmap);
        }

        exit(0);
    }
}
