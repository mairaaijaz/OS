#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    // uptime doesn't take any arguments
    if (argc > 1) {
        fprintf(2, "Usage: uptime\n");
        exit(1);
    }

    // Call the system to get the current ticks
    int ticks = uptime();
    printf("%d\n", ticks);

    exit(0);
}
