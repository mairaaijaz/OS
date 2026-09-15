#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

// --- Regular Expression Functions (borrowed from grep.c) ---
int matchhere(char*, char*);
int matchstar(int, char*, char*);

int match(char *re, char *text) {
  if (re[0] == '^')
    return matchhere(re+1, text);
  do { 
    if (matchhere(re, text))
      return 1;
  } while (*text++ != '\0');
  return 0;
}

int matchhere(char *re, char *text) {
  if (re[0] == '\0')
    return 1;
  if (re[1] == '*')
    return matchstar(re[0], re+2, text);
  if (re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if (*text != '\0' && (re[0] == '.' || re[0] == *text))
    return matchhere(re+1, text+1);
  return 0;
}

int matchstar(int c, char *re, char *text) {
  do { 
    if (matchhere(re, text))
      return 1;
  } while (*text != '\0' && (*text++ == c || c == '.'));
  return 0;
}
// ---------------------------------------------------------

void find(char *path, char *target, int is_exec, char *exec_argv[]) {
    char buf[512];
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_DIR) {
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long\n");
            close(fd);
            return;
        }
        
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            
            if (match(target, de.name)) {
                if (is_exec) {
                    if (fork() == 0) {
                        int count = 0;
                        while(exec_argv[count] != 0) count++;
                        
                        char *new_argv[MAXARG];
                        for(int i = 0; i < count; i++){
                            new_argv[i] = exec_argv[i];
                        }
                        new_argv[count] = buf; // Add the matched file as the last argument
                        new_argv[count + 1] = 0;
                        
                        exec(new_argv[0], new_argv);
                        fprintf(2, "find: exec failed\n");
                        exit(1);
                    } else {
                        wait(0);
                    }
                } else {
                    printf("%s\n", buf);
                }
            }
            
            struct stat st_child;
            if (stat(buf, &st_child) >= 0 && st_child.type == T_DIR) {
                find(buf, target, is_exec, exec_argv);
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(2, "Usage: find <path> <filename/regex> [-exec cmd...]\n");
        exit(1);
    }
    
    int is_exec = 0;
    char *exec_argv[MAXARG];
    
    if (argc >= 4 && strcmp(argv[3], "-exec") == 0) {
        is_exec = 1;
        int idx = 0;
        for (int i = 4; i < argc; i++) {
            exec_argv[idx++] = argv[i];
        }
        exec_argv[idx] = 0;
    }
    
    find(argv[1], argv[2], is_exec, exec_argv);
    exit(0);
}
