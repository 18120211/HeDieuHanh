#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main(){
    int file, random_number;
    file = open("/dev/random_module", O_RDONLY);
    if(file == -1)
    {
        printf("Failed to open file\n");
        return 0;
    }
    read(file, &random_number, sizeof(random_number));
    printf("Random number : %d\n", random_number);
    return 0;
}
