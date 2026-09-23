// sandbox: run a command with a syscall mask
// Usage: sandbox <mask> <allowed_path> <cmd> [args...]
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 4) {
    fprintf(2, "usage: sandbox <mask> <path> <cmd> [args...]\n");
    exit(1);
  }

  int mask = atoi(argv[1]);
  char *path = argv[2];
  char *cmd = argv[3];
  char **args = &argv[3];

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "sandbox: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // child: set sandbox mask then exec
    if (interpose(mask, path) < 0) {
      fprintf(2, "sandbox: interpose failed\n");
      exit(1);
    }
    exec(cmd, args);
    fprintf(2, "sandbox: exec %s failed\n", cmd);
    exit(1);
  }

  // parent: wait for child
  wait(0);
  exit(0);
}
