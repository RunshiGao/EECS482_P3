#include "vm_pager.h"
#include <queue>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

class page_entry {
public:
    page_entry() : type(0), block(-1), referenced(false), valid(false), dirty(false) {

    }
    string filename;
    int type;     // 0 means swapped based, 1 means file based
    int block;     // -1 if it is not assigned to a block for (swap-based)
    bool referenced;
    bool valid;
    bool resident;
    bool dirty;
};

class process_mem {
public:
    process_mem() : available_vm_index(0) {
        memset(hw_page_table.ptes, 0, VM_ARENA_SIZE/VM_PAGESIZE * sizeof(page_table_entry_t));
    }
    // the num of currently swap-based
    unsigned int num_swap_based;
    pid_t pid;
    page_table_t hw_page_table;
    page_entry os_page_table[VM_ARENA_SIZE / VM_PAGESIZE];
    unsigned int available_vm_index;
    // this is the one function for calculating hw bits given os bits
    void set_hw(int vp_index);
    pair<string,int> convert(const char* filename);
};

// get the free physical page from physical memory, return the index of pp
int get_ppage();
// evict one pp from memory, return index of pp
int evict();
void write_page_to_disk_file(string filename, int block);
void write_page_to_disk_swap(int block);
void write_page_file(int ppage);
void read_page_file(int ppage);
void read_page(int ppage);