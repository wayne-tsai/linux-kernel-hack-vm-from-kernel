//2.6.39.4 x86_64
//繳交 test.c myproc.c binary and source
#include <stdio.h>
#include <unistd.h>
int main()
{
  int a=5;
  int b=6;
  int c=7;
  int d=8;
  printf("%d\n",getpid());
  while(1){
    printf("%d\n", a);
    printf("%p\n", &a);
    sleep(5);
  }
    return 0;
}
