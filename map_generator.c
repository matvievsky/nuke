#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define ERROR_HANDLER(x) ErrorHandler(x)
void ErrorHandler(const char *error_text);

int main(const int argc, const char* argv[]) {
    if (argc > 3) {
        ERROR_HANDLER("Wrong number of arguments");
    }
    const unsigned int n = argc >= 2 ? atoi(argv[1]) : 10;
    const unsigned int GRID_SIZE = argc >= 3 ? atoi(argv[2]) : 100;
    srand(getpid());
    FILE *stream = fopen("coords.txt", "w");
    for (unsigned int i = 0; i < n; ++i) {
        fprintf(stream, "%u,%u\n", rand() % GRID_SIZE, rand() % GRID_SIZE);
    }
    fclose(stream);
    printf("Map of %u target(s) with size = %d enerated successfully\n", n, GRID_SIZE);
    exit(EXIT_SUCCESS);
}

void ErrorHandler(const char *error_text) {
    errno = 1;
    perror(error_text);
    exit(EXIT_FAILURE);
}

