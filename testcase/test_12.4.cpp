#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"

using std::cout;

int main()
{
    /* Allocate swap-backed page from the arena */
    char *filename = (char *) vm_map(nullptr, 0);

    /* Write the name of the file that will be mapped */
    strcpy(filename, "lampson83.txt");

    if(fork() == 0) {
        strcpy(filename, "abc.txt");
        vm_yield();
    } else {
        vm_yield();
        strcpy(filename, "dddd.txt");
    }
}