#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *fp = fopen("badfile", "wb");  
    if (!fp) {
        perror("fopen");
        return 1;
    }


    for (int i = 1; i <= 300; i++) {
        unsigned char byte = (unsigned char)(i % 255);
        if (byte == 0) byte = 255;  
        fputc(byte, fp);
    }

    fclose(fp);
    return 0;
}
