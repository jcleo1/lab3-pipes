cat << 'EOF' > pipes_processes3.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        exit(1);
    }

    int pipe1[2]; // cat -> grep
    int pipe2[2]; // grep -> sort

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        exit(1);
    }

    int pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }

    if (pid == 0) {
        // Child process (P2)
        int pid2 = fork();

        if (pid2 < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid2 == 0) {
            // Child's child (P3: executes 'sort')
            // Read from pipe2, standard output to screen
            dup2(pipe2[0], 0);

            // Close all unused ends in P3
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            char *sort_args[] = {"sort", NULL};
            execvp("sort", sort_args);
            perror("execvp sort failed");
            exit(1);
        } else {
            // Child (P2: executes 'grep <arg>')
            // Read from pipe1, write to pipe2
            dup2(pipe1[0], 0);
            dup2(pipe2[1], 1);

            // Close all unused ends in P2
            close(pipe1[0]);
            close(pipe1[1]);
            close(pipe2[0]);
            close(pipe2[1]);

            char *grep_args[] = {"grep", argv[1], NULL};
            execvp("grep", grep_args);
            perror("execvp grep failed");
            exit(1);
        }
    } else {
        // Parent process (P1: executes 'cat scores')
        // Standard output redirected to pipe1
        dup2(pipe1[1], 1);

        // Close all unused ends in P1
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);

        char *cat_args[] = {"cat", "scores", NULL};
        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    return 0;
}
EOF