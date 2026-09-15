#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target) {
    char buf[512];
    char *p;
    int fd;
    struct dirent de;
    struct stat st;

    // Try to open the current path
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // Get statistics about the file/folder we just opened
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // We only want to search if this is a directory
    if (st.type == T_DIR) {
        // Prevent buffer overflow if the path gets too crazy long
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long\n");
            close(fd);
            return;
        }
        
        // Setup the buffer to look like: "path/"
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        
        // Read directory entries one by one
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            // Skip empty entries
            if (de.inum == 0)
                continue;
            
            // Skip the special "." and ".." directories to prevent infinite loops!
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            
            // Append the file name to our path buffer so it looks like: "path/filename"
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0; // Null-terminate the string
            
            // If the name matches our target, print the full path!
            if (strcmp(de.name, target) == 0) {
                printf("%s\n", buf);
            }
            
            // Check if this new path is a directory itself. If so, search inside it recursively!
            struct stat st_child;
            if (stat(buf, &st_child) >= 0 && st_child.type == T_DIR) {
                find(buf, target);
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    // Make sure the user provided exactly a path and a target filename
    if (argc != 3) {
        fprintf(2, "Usage: find <path> <filename>\n");
        exit(1);
    }
    
    // Start searching
    find(argv[1], argv[2]);
    exit(0);
}
