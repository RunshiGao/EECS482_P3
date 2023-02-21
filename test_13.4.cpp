#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;

int main()
{
    char *f1 = (char*)vm_map(nullptr, 0);
    char *f2 = (char*)vm_map(nullptr, 0);
    f1[65535] = 'd'; f2[0] = 'a'; f2[1] = 't';f2[2] = 'a';
    f2[3] = '1';f2[4] = '.';f2[5] = 'b';f2[6] = 'i';
    f2[7] = 'n';f2[8] = '\0';
    char *file = (char*)vm_map( f1 + 65535, 0 );
    file[0] = 'i';
    file[1] = 'a';
    char* tmp = (char*)vm_map(nullptr,0);
    *tmp = 'a';
    file = (char*)vm_map( f1 + 65535, 0 );
    *tmp = 'a';
    file = (char*)vm_map( f1 + 65535, 0 );
    char* f3 = (char*)vm_map(f1 + 65535, 0);
    char* f4 = (char*)vm_map(f1 + 65535, 1);
    f3[65535] = 'd';f4[0] = 'a';f4[1] = 't';f4[2] = 'a';f4[3] = '2';
    f4[4] = '.';f4[5] = 'b';f4[6] = 'i';f4[7] = 'n';f4[8] = '\0';
    file = (char*)vm_map( f3 + 65535, 0 );
    *tmp = 'a';
    file = (char*)vm_map( f3 + 65535, 0 );
    file[0] = 'b';
    file[1] = 'a';
    cout<<file[0]<<endl;
}