#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;
#include <string> 

int main()
{
    /* Allocate swap-backed page from the arena */
    char *str1 = (char *) vm_map(nullptr, 0);
    char *str2 = (char *) vm_map(nullptr, 0);
    char *str3 = (char *) vm_map(nullptr, 0);
    /* Write the name of the file that will be mapped */
    strcpy(str1, "a.txt");
    printf("%s\n",str1);
    strcpy(str2, "b.txt");
    printf("%s\n",str2);
    strcpy(str3, "c.txt");
    printf("%s\n",str3);
    if(fork()) {
        char *str7 = (char *) vm_map(str3, 0);         // not exist
        cout<<str7[0]<<endl;
        strcpy(str1, "a.txt");
        printf("%s\n",str1);
        strcpy(str2, "b.txt");
        printf("%s\n",str2);
        strcpy(str3, "c.txt");
        printf("%s\n",str3);
        exit(0);
    } else {
        char *str7 = (char *) vm_map(str3, 0);         // not exist
        cout<<str7[0]<<endl;
        strcpy(str1, "a.txt");
        printf("%s\n",str1);
        strcpy(str2, "b.txt");
        printf("%s\n",str2);
        strcpy(str3, "c.txt");
        printf("%s\n",str3);
        exit(0);
    }
}