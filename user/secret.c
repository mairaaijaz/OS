// secret: write a secret string to memory, then exit (freeing pages)
// Used to test the attack program.
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(2, "usage: secret <string>\n");
    exit(1);
  }

  // allocate a page and write the secret into it
  char *p = sbrk(4096);
  if (p == (char *)-1) {
    fprintf(2, "secret: sbrk failed\n");
    exit(1);
  }

  // copy secret to the allocated page
  int i;
  for (i = 0; argv[1][i] != '\0' && i < 4095; i++)
    p[i] = argv[1][i];
  p[i] = '\0';

  exit(0);
}
