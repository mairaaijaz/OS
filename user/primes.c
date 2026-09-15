#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void prime_sieve(int left_pipe_read) {
    int p;
    // Read the first number from the left neighbor
    if (read(left_pipe_read, &p, sizeof(p)) == 0) {
        return; // (CHANGED: use return instead of exit)
    }

    printf("prime %d\n", p);

    int p2c[2];
    pipe(p2c);

    int pid = fork();

    if (pid == 0) {
        // ------------- CHILD PROCESS -------------
        close(p2c[1]); 
        prime_sieve(p2c[0]); 
        return; // (CHANGED)
    } else {
        // ------------- PARENT PROCESS -------------
        close(p2c[0]); 
        int n;
        
        while (read(left_pipe_read, &n, sizeof(n)) > 0) {
            if (n % p != 0) {
                write(p2c[1], &n, sizeof(n));
            }
        }
        
        close(left_pipe_read); 
        close(p2c[1]); 
        wait(0); 
        return; // (CHANGED)
    }
}

int main(int argc, char *argv[]) {
    int p2c[2];
    pipe(p2c);

    int pid = fork();

    if (pid == 0) {
        close(p2c[1]); 
        prime_sieve(p2c[0]); 
        exit(0); // Exit is fine here in main!
    } else {
        close(p2c[0]); 
        for (int i = 2; i <= 35; i++) {
            write(p2c[1], &i, sizeof(i));
        }
        close(p2c[1]); 
        wait(0); 
        exit(0);
    }
    return 0;
}
