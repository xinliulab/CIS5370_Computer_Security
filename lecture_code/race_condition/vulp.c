#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
   char * fn = "/tmp/XYZ";
   char buffer[60];
   FILE *fp;

   /* get user input */
   scanf("%50s", buffer);

   if (!access(fn, W_OK)) {
      fp = fopen(fn, "a+");
      if (fp == NULL) {
          perror("fopen failed");
          return 1;
      }
      fwrite("\n", sizeof(char), 1, fp);
      fwrite(buffer, sizeof(char), strlen(buffer), fp);
      fclose(fp);
  }
   else printf("No permission \n");

   return 0;
}
