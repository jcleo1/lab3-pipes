#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    // fd1: P1 (Parent) -> P2 (Child)
    // fd2: P2 (Child) -> P1 (Parent)
    int fd1[2];
    int fd2[2];

    char fixed_str1[] = "howard.edu";
    char fixed_str2[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe 1 Failed\n");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe 2 Failed\n");
        return 1;
    }

    printf("Input : ");
    scanf("%s", input_str);

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed\n");
        return 1;
    }

    // Parent process (P1)
    else if (p > 0) {
        char concat_str[200];

        // Close unused ends
        close(fd1[0]); // Don't read from fd1
        close(fd2[1]); // Don't write to fd2

        // Write initial input to P2
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Wait for child to finish sending back
        wait(NULL);

        // Read the returned string from P2
        read(fd2[0], concat_str, sizeof(concat_str));
        close(fd2[0]);

        // Concatenate "gobison.org"
        int k = strlen(concat_str);
        int i;
        for (i = 0; i < strlen(fixed_str2); i++) {
            concat_str[k++] = fixed_str2[i];
        }
        concat_str[k] = '\0';

        // Print final output
        printf("Output : %s\n", concat_str);
    }

    // Child process (P2)
    else {
        char concat_str[200];
        char second_input[100];

        // Close unused ends
        close(fd1[1]); // Don't write to fd1
        close(fd2[0]); // Don't read from fd2

        // Read string sent by P1
        read(fd1[0], concat_str, sizeof(concat_str));
        close(fd1[0]);

        // Concatenate "howard.edu"
        int k = strlen(concat_str);
        int i;
        for (i = 0; i < strlen(fixed_str1); i++) {
            concat_str[k++] = fixed_str1[i];
        }
        concat_str[k] = '\0';

        // Print intermediate output
        printf("Output : %s\n", concat_str);

        // Prompt for second input
        printf("Input : ");
        scanf("%s", second_input);

        // Append second input to the existing string
        k = strlen(concat_str);
        for (i = 0; i < strlen(second_input); i++) {
            concat_str[k++] = second_input[i];
        }
        concat_str[k] = '\0';

        // Send back to P1 via fd2
        write(fd2[1], concat_str, strlen(concat_str) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}