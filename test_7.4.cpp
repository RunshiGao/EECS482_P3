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
    strcpy(filename, "data1.bin");

    /* Map a page from the specified file */
    char *p1 = (char *) vm_map (filename, 0);
    
    
    if(fork()==0){
        strcpy(p1,"!!!!!!");
        for (unsigned int i=0; i<20; i++) {
            cout << p1[i];
        }
    } else {
        for (unsigned int i=0; i<20; i++) {
            cout << p1[i];
        }
    }
}