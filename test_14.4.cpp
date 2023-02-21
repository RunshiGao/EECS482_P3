#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;

int main(){
    for (int i = 0; i < 5; i++) {
        int pid1 = fork();
        char *p1 = (char *) vm_map(nullptr, 0);
        for (unsigned int j = 60000; j < 60100; j++) {
            if (j % 3 == 0) {
                p1[j] = i;
            } else if (j % 3 == 1) {
                p1[j] = j;
            } else {
                p1[j] = pid1;
            }
            vm_yield();
            cout << p1[j];
        }
    }
    return 0;
}
