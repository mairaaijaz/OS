#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int parent_to_child[2]; // Pipe for Parent to send to Child
  int child_to_parent[2]; // Pipe for Child to send to Parent
  char buffer[1];         // A 1-byte buffer to hold our "ping"

  // Create the two pipes
  pipe(parent_to_child);
  pipe(child_to_parent);

  // Create the child process
  int pid = fork();

  if(pid == 0) {
    // ---------------- CHILD PROCESS ----------------
    close(parent_to_child[1]); // Close unused ends of the pipes
    close(child_to_parent[0]); 

    // Read the byte from the parent
    read(parent_to_child[0], buffer, 1);
    
    // Print the required message with the child's ID
    printf("%d: received ping\n", getpid());
    
    // Send the byte back to the parent
    write(child_to_parent[1], buffer, 1);
    
    // Exit successfully
    exit(0);
  } else {
    // ---------------- PARENT PROCESS ----------------
    close(parent_to_child[0]); // Close unused ends of the pipes
    close(child_to_parent[1]);

    // Send a byte to the child
    write(parent_to_child[1], "A", 1);
    
    // Wait to read the byte back from the child
    read(child_to_parent[0], buffer, 1);
    
    // Print the required message with the parent's ID
    printf("%d: received pong\n", getpid());
    
    // Exit successfully
    exit(0);
  }
}
