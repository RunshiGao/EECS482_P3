#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *str1 = (char *) vm_map(nullptr, 0);
    char *str2 = (char *) vm_map(nullptr, 0);
    char *str3 = (char *) vm_map(nullptr, 0);
    char *str4 = (char *) vm_map(nullptr, 0);
    char *str5 = (char *) vm_map(nullptr, 0);
    char *str6 = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(str1, "a.txt");
    printf("%s\n",str1);
    strcpy(str2, "b.txt");
    printf("%s\n",str2);
    strcpy(str3, "c.txt");
    printf("%s\n",str3);
    strcpy(str4, "d.txt");
    printf("%s\n",str4);
    strcpy(str5, "e.txt");
    printf("%s\n",str5);
    strcpy(str6, "f.txt");
    printf("%s\n",str6);
    printf("%s\n",str1);
    printf("%s\n",str5);
    printf("%s\n",str6);
    printf("%s\n",str2);
    exit(0);
}