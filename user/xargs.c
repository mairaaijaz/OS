#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h" // For MAXARG

int main(int argc, char *argv[]) {
    // We need at least a command to run
    if (argc < 2) {
        fprintf(2, "Usage: xargs <command>\n");
        exit(1);
    }

    char *cmd_args[MAXARG];
    int base_args = 0;
    
    // Copy the original command and its initial arguments
    for (int i = 1; i < argc; i++) {
        cmd_args[base_args++] = argv[i];
    }

    char buf[1024];
    int buf_pos = 0;
    char c;

    // Read from standard input (FD 0) one byte at a time
    while (read(0, &c, 1) > 0) {
        if (c == '\n') {
            buf[buf_pos] = '\0'; // End of the line
            
            // Split the line by spaces to get individual arguments
            int curr_arg = base_args;
            char *p = buf;
            
            // Skip leading spaces
            while (*p == ' ') p++;
            
            if (*p != '\0') {
                cmd_args[curr_arg++] = p; // Grab the first argument
                
                // Find spaces and replace them with '\0' to split the string
                while (*p != '\0') {
                    if (*p == ' ') {
                        *p = '\0'; 
                        p++;
                        while (*p == ' ') p++; 
                        if (*p != '\0') {
                            cmd_args[curr_arg++] = p; 
                        }
                    } else {
                        p++;
                    }
                }
            }
            
            cmd_args[curr_arg] = 0; // Null-terminate the argument array

            // Create a child process to actually run the command
            if (fork() == 0) {
                exec(cmd_args[0], cmd_args);
                fprintf(2, "xargs: exec failed\n");
                exit(1);
            } else {
                wait(0); // Parent waits for the child to finish
            }
            
            // Reset the buffer for the next line
            buf_pos = 0;
        } else {
            buf[buf_pos++] = c;
        }
    }

    exit(0);
}
