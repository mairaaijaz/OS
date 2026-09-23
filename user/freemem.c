#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  printf("Free memory: %d bytes\n", (int)freemem());
  exit(0);
}
