#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>
int main()
{ 
    int a = 10;
    int b = 20;
    int ret=syscall(78,a,b); //after modify syscall 78
    printf("%d\n",ret);
    return 0;
}