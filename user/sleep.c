#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // If the user didn't provide exactly one argument, print an error
  if(argc != 2){
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1);
  }

  // Convert the string argument to an integer
  int ticks = atoi(argv[1]);
  
  // Call the system's sleep command
  sleep(ticks);

  // Exit successfully
  exit(0);
}
