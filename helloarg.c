#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("helloarg: arguments passed in: ");
    int i;
    for (i = 1; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}
