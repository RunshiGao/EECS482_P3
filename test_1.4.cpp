#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *str1 = (char *) vm_map(nullptr, 0);
    char *str2 = (char *) vm_map(nullptr, 0);
    char *str3 = (char *) vm_map(nullptr, 0);
    strcpy(str1,"a");
    strcpy(str2,"b");
    strcpy(str3,"c");
    
    if(fork()==0){
        strcpy(str1,"d");
        
        strcpy(str3,"f");
        printf("Child str1:%s\n",str1);
        printf("Child str2:%s\n",str2);
        
        printf("Child str3:%s\n",str3);
        exit(0);
    } else {
        printf("Parent str1:%s\n",str1);
        printf("Parent str2:%s\n",str2);
        
        printf("Parent str3:%s\n",str3);
        vm_yield();
        exit(0);
    }
}