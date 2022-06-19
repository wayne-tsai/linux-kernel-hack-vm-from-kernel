//2.6.39.4 x86_64
//繳交 test.c myproc.c binary and source
#include <stdio.h>
#include <unistd.h>
int main()
{
  int a=8;
  int b=6;
  int c=7;
  int d=8;
  int counter=0;
  void* p=&a;
  printf("%d\n",getpid());
  while(1){
    printf("Counter: %d\n", ++counter);
    printf("Value: %d\n", a);
    printf("Address: %p\n", &a);
    printf("Addr p point to: %p\n", p);
    printf("Value p point to: %d\n", *(int*)p);	
    sleep(5);
  }
    return 0;
}
