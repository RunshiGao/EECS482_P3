#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    char *str6 = (char *) vm_map(nullptr, 0);
    strcpy(str6,"data1.bin");
    char *str7 = (char *) vm_map (str6, 5);
    char *str1 = (char *) vm_map(nullptr, 0);
    char *str2 = (char *) vm_map(nullptr, 0);
    char *str3 = (char *) vm_map(nullptr, 0);
    char *str4 = (char *) vm_map(nullptr, 0);
    char *str5 = (char *) vm_map(nullptr, 0);
    
    strcpy(str1,"a");
    strcpy(str2,"b");
    strcpy(str3,"c");
    strcpy(str4,"d");
    strcpy(str5,"e");
    int ret = fork();
    if(ret == 0){
        strcpy(str1,"x");
        strcpy(str3,"y");
        strcpy(str5,"z");
        strcpy(str7,"!!!!!!");
        printf("Child str1:%s\n",str1);
        printf("Child str2:%s\n",str2);
        printf("Child str3:%s\n",str3);
        printf("Child str4:%s\n",str4);
        printf("Child str5:%s\n",str5);
        for (unsigned int i=0; i<30; i++) 
	        cout << str7[i];

        
    } else {
        printf("Parent str1:%s\n",str1);
        printf("Parent str2:%s\n",str2);
        printf("Parent str3:%s\n",str3);
        printf("Parent str4:%s\n",str4);
        printf("Parent str5:%s\n",str5);
        vm_yield();
        for (unsigned int i=0; i<30; i++) 
	        cout << str7[i];
    }
    for (unsigned int i=0; i<30; i++) 
	    cout << str7[i];
}