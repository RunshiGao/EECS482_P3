#include <iostream>
#include <cstring>
#include <unistd.h>
#include "vm_app.h"
using namespace std;

int main(){
    char* filename1 = (char*) vm_map(nullptr, 0);
    char* filename2 = (char*) vm_map(nullptr, 0);
    strcpy(filename1, "data1.bin");
    strcpy(filename2, "data2.bin");
    char* block1 = (char*) vm_map(filename1, 1);
    char* block2 = (char*) vm_map(filename1, 2);
    char* block3 = (char*) vm_map(filename1, 3);
    char* block4 = (char*) vm_map(filename1, 4);
    char* block5 = (char*) vm_map(filename1, 1);
    char* block6 = (char*) vm_map(filename1, 2);
    char* block7 = (char*) vm_map(filename1, 3);
    char* block8 = (char*) vm_map(filename1, 4);
    char* block9 = (char*) vm_map(filename2, 1);
    char* block10 = (char*) vm_map(filename2, 2);
    char* block12 = (char*) vm_map(filename2, 4);
    char* block13 = (char*) vm_map(filename2, 1);
    char* block14 = (char*) vm_map(filename2, 2);
    char* block15 = (char*) vm_map(filename2, 3);
    char* block16 = (char*) vm_map(filename2, 4);
    int pid = fork();
    if(!pid){
        cout << *filename1 << endl;
        cout << *filename2 << endl;
        char* block11 = (char*) vm_map(filename1, 1);
        char* block22 = (char*) vm_map(filename1, 2);
        char* block33 = (char*) vm_map(filename1, 3);
        cout << *block11 << *block22 << *block33 << endl;
    }
    else{
        int pid2 = fork();
        if( !pid2 ){
            char* block11 = (char*) vm_map(filename1, 1);
            char* block22 = (char*) vm_map(filename1, 2);
            char* block33 = (char*) vm_map(filename1, 3);
            cout<<*filename1<<endl;
            char* block44 = (char*) vm_map(filename1, 4);
            cout << *block11 << *block22 << *block33 << *block44 << endl;
            block44 = (char*) vm_map(filename1, 5);
            cout<< *block4 << endl;
            cout<< *block5 << endl;
            cout << *block1 << endl;
            cout<< *block4 << endl;
            cout<< *block5 << endl;
            cout << *block11 << endl;
            cout << *block12 << endl;
            cout << *block13 << endl;
            cout << *block14 << endl;
            cout << *block15 << endl;
            cout << *block16 << endl;
            cout<<*filename2<<endl;
        }else{
            char* block5 = (char*) vm_map(filename1, 5);
            cout<< *block1 << endl;
            *block1 = 'a';
            cout<< *block2 << endl;
            *block3 = 'a';
            cout<< *block4 << endl;
            cout<< *block5 << endl;
            cout << *block1 << endl;
            cout << *block2 << endl;
            *block3 = 'a';
            *block4 = 'a';
            *filename1 = 'a';
            cout << *filename1 << endl;
            cout << *block5 << endl;
            *block3 = 'a';
            *block4 = 'a';
            *block5 = 'a';
            *block6 = 'a';
            *block7 = 'a';
            *filename1 = 'b';
            *block8 = 'a';
            *block9 = 'a';
            *block10 = 'a';
            *filename2 = 'a';
            cout<<*filename2<<endl;
        }
    }
    return 0;
}