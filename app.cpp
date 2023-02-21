#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using std::cout;
using std::endl;

// const unsigned int VM_PAGESIZE = 65536;

void modify_vp(int vpage, char* str_addr, uint offset, char* msg){
    char *filename = str_addr + VM_PAGESIZE * vpage + offset;
    strcpy(filename, msg);
}


void print_vp(char* vpage){
    for (unsigned int i=0; i<5; i++) {
	cout << vpage[i];
    }
}


int main() {
    if (fork() == 0) {
        cout << "child process runs" << endl;
        char *page0 = (char *) vm_map(nullptr, 0);
        char *page1 = (char *) vm_map(nullptr, 0);
        char *filename = page0 + VM_PAGESIZE - 4;  //on vpage 0, 1
        strcpy(filename, "lampson83.txt");
        char *page2 = (char *) vm_map(nullptr, 0);
        char *page3 = (char *) vm_map(filename, 0);
        vm_yield();
        cout << "switch back to child" << endl;
        // char *page4 = (char *) vm_map(nullptr, 0);
        page2[0] = 'k';
        cout << "page2 write" << endl;
        page3[0] = 'p';
        cout << "page3 write" << endl;
        // page4[0] = 'q';
        // cout << "page4 write" << endl;
        strcpy(filename, "ksdsdsdsd.txt");
        for (int i=0; i<12; i++){
            cout << filename[i] << endl;
        }
    }
    else{
        printf("parent process running\n");
        char *page0 = (char *) vm_map(nullptr, 0);
        char *page1 = (char *) vm_map(nullptr, 0);
        char *filename = page0 + VM_PAGESIZE - 4;  //on vpage 0, 1
        strcpy(filename, "data2.bin");
        vm_yield();
        printf("switch back to parent\n");
        char *page2 = (char *) vm_map(nullptr, 0);
        char *page3 = (char *) vm_map(nullptr, 0);
        char *page4 = (char *) vm_map(filename, 0);
        char *page5 = (char *) vm_map(filename, 0);
        cout << "before" << endl;
        page2[0] = 'p';
        cout << "after" << endl;
        page3[0] = 'q';
        cout << "page3 write" << endl;
        page4[0] = 'g';
        cout << "page4 write" << endl;
        page5[0] = 'e';
        cout << "page5 write" << endl;
        strcpy(filename, "uijjdscs.txt");
        for (int i=0; i<13; ++i){
            cout << filename[i] << endl;
        }
    }
}