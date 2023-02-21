#include "page_structure.h"
#include <queue>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

extern unordered_map<int, process_mem*> pid_mem;
extern process_mem* current_process;
extern queue<int> free_phys_mem;
extern queue<int> free_swap_block;
extern queue<int> clock_queue;
extern unordered_map<int, unordered_map<int, int>> pp2vp_swap;
extern unordered_map<int, unordered_map<int, int>> block2vp_swap;
extern unordered_map<int, unordered_map<int, vector<int>>> pp2vp_file;
extern unordered_map<string, unordered_map<int, unordered_map<int, vector<int>>>> block2vp_file;
extern unordered_map<int, pair<pair<string, int>, pair<bool, bool>>> ready2evict_file;
extern unordered_map<string, unordered_map<int, int>> block2pp_file;

// this is the one function for calculating hw bits given os bits
void process_mem::set_hw(int vp_index) {
    page_entry entry = os_page_table[vp_index];
    if (!entry.resident || !entry.referenced) {
        hw_page_table.ptes[vp_index].read_enable = 0;
        hw_page_table.ptes[vp_index].write_enable = 0;
    } else {
        if (entry.dirty) {
            hw_page_table.ptes[vp_index].read_enable = 1;
            hw_page_table.ptes[vp_index].write_enable = 1;
        } else {
            hw_page_table.ptes[vp_index].read_enable = 1;
            hw_page_table.ptes[vp_index].write_enable = 0;
        }
    }
}

pair<string,int> process_mem::convert(const char *filename) {
    int flag = vm_fault(filename, false);
    if(flag == -1) {
        return {"", -1};
    }
    unsigned int offset = ((uintptr_t)filename-(uintptr_t)VM_ARENA_BASEADDR) % VM_PAGESIZE;
    unsigned int virtual_index = ((uintptr_t)filename-(uintptr_t)VM_ARENA_BASEADDR) / VM_PAGESIZE;
    unsigned int physical_index = hw_page_table.ptes[virtual_index].ppage;
    char *physical_address = (char*)vm_physmem + physical_index * VM_PAGESIZE + offset;
    string res = "";
    while(*physical_address != '\0') {
        res += *physical_address;
        filename++;
        int flag = vm_fault(filename, false);
        if(flag == -1) {
            return {"", -1};
        }
        offset = ((uintptr_t)filename-(uintptr_t)VM_ARENA_BASEADDR) % VM_PAGESIZE;
        virtual_index = ((uintptr_t)filename-(uintptr_t)VM_ARENA_BASEADDR) / VM_PAGESIZE;
        physical_index = hw_page_table.ptes[virtual_index].ppage;
        physical_address = (char*)vm_physmem + physical_index * VM_PAGESIZE + offset;
    }
    return {res, 0};
}

void read_page(int ppage) {
    // it is shared?
    bool cow = pp2vp_swap[ppage].size() > 1;
    for(auto m: pp2vp_swap[ppage]) {
        process_mem* p = pid_mem[m.first];
        int vp_index = m.second;
        p->os_page_table[vp_index].resident = true;
        p->os_page_table[vp_index].referenced = true;
        p->hw_page_table.ptes[vp_index].ppage = ppage;
        if (cow) {
            p->hw_page_table.ptes[vp_index].read_enable = 1;
            p->hw_page_table.ptes[vp_index].write_enable = 0;
        } else {
            p->set_hw(vp_index);
        }
    }
}

void read_page_file(int ppage) {
    for(auto m: pp2vp_file[ppage]) {
        process_mem* p = pid_mem[m.first];
        for (auto vp_index: m.second) {
            p->os_page_table[vp_index].resident = true;
            p->os_page_table[vp_index].referenced = true;
            p->hw_page_table.ptes[vp_index].ppage = ppage;
            p->set_hw(vp_index);
        }
    }
}

void write_page_file(int ppage) {
    for(auto m: pp2vp_file[ppage]) {
        process_mem* p = pid_mem[m.first];
        for (auto vp_index: m.second) {
            p->os_page_table[vp_index].resident = true;
            p->os_page_table[vp_index].referenced = true;
            p->os_page_table[vp_index].dirty = true;
            p->hw_page_table.ptes[vp_index].ppage = ppage;
            p->set_hw(vp_index);
        }
    }
}

void write_page_to_disk_swap(int block) {
    for(auto m: block2vp_swap[block]) {
        process_mem* p = pid_mem[m.first];
        int vp_index = m.second;
        p->os_page_table[vp_index].resident = false;
        p->os_page_table[vp_index].referenced = false;
        p->os_page_table[vp_index].dirty = false;
        p->os_page_table[vp_index].block = block;
        p->set_hw(vp_index);
    }
}

void write_page_to_disk_file(string filename, int block) {
    for(auto m: block2vp_file[filename][block]) {
        process_mem* p = pid_mem[m.first];
        for (auto vp_index: m.second) {
            p->os_page_table[vp_index].resident = false;
            p->os_page_table[vp_index].referenced = false;
            p->os_page_table[vp_index].dirty = false;
            p->os_page_table[vp_index].block = block;
            p->os_page_table[vp_index].filename = filename;
            p->set_hw(vp_index);
        }
    }
}

// evict one pp from memory, return index of pp
int evict() {
    // use clock replacement policy
    while(1) {
        int ppage = clock_queue.front();
        clock_queue.pop();
        if(ready2evict_file.find(ppage) != ready2evict_file.end()) {
            auto &pai = ready2evict_file[ppage];
            if (pai.second.first) {
                pai.second.first = false;
                clock_queue.push(ppage);
            } else {
                if (pai.second.second) {
                    int flag = file_write(pai.first.first.c_str(), pai.first.second, (char*)vm_physmem + ppage * VM_PAGESIZE);
                    if (flag == -1) {
                        return -1;
                    }
                }
                block2pp_file[pai.first.first].erase(pai.first.second);
                ready2evict_file.erase(ppage);
                return ppage;
            }
        } else if (pp2vp_swap[ppage].size() != 0) {
            int first_pid = pp2vp_swap[ppage].begin()->first;
            int first_vp_index = pp2vp_swap[ppage].begin()->second;
            process_mem* ptr = pid_mem[first_pid];
            bool referenced = ptr->os_page_table[first_vp_index].referenced;
            bool dirty = ptr->os_page_table[first_vp_index].dirty;
            int block = ptr->os_page_table[first_vp_index].block;
            if(referenced) {
                // if it is referenced
                for(auto m: pp2vp_swap[ppage]) {
                    process_mem* p = pid_mem[m.first];
                    int vp_index = m.second;
                    p->os_page_table[vp_index].referenced = false;
                    p->set_hw(vp_index);
                }
                // push back to clock queue
                clock_queue.push(ppage);
            } else {
                // if it is not referenced, evict it
                if(dirty) {
                    // if it is dity, write back to swap file
                    // error handling, return -1
                    int flag = file_write(nullptr, block, (char*)vm_physmem + ppage * VM_PAGESIZE);
                    if (flag == -1) {
                        return -1;
                    }
                } 
                block2vp_swap[block] = pp2vp_swap[ppage];
                pp2vp_swap.erase(ppage);
                write_page_to_disk_swap(block);
                return ppage;
            }
        } else if (pp2vp_file[ppage].size() != 0) {
            int first_pid = pp2vp_file[ppage].begin()->first;
            int first_vp_index = pp2vp_file[ppage].begin()->second[0];
            process_mem* ptr = pid_mem[first_pid];
            bool referenced = ptr->os_page_table[first_vp_index].referenced;
            bool dirty = ptr->os_page_table[first_vp_index].dirty;
            int block = ptr->os_page_table[first_vp_index].block;
            string filename = ptr->os_page_table[first_vp_index].filename;
            if (referenced) {
                for(auto m: pp2vp_file[ppage]) {
                    process_mem* p = pid_mem[m.first];
                    for (auto vp_index: m.second) {
                        p->os_page_table[vp_index].referenced = false;
                        p->set_hw(vp_index);
                    }
                }
                clock_queue.push(ppage);
            } else {
                if(dirty) {
                    int flag = file_write(filename.c_str(), block, (char*)vm_physmem + ppage * VM_PAGESIZE);
                    if (flag == -1) {
                        return -1;
                    }
                }
                pp2vp_file.erase(ppage);
                write_page_to_disk_file(filename, block);
                block2pp_file[filename].erase(block);
                return ppage;
            }
        }
    }
}

// get the free physical page from physical memory, return the index of pp
int get_ppage() {
    if(!free_phys_mem.empty()) {
        int ppage = free_phys_mem.front();
        free_phys_mem.pop();
        // clock_queue.push(ppage);
        return ppage;
    } else {
        int ppage = evict();
        // clock_queue.push(ppage);
        return ppage;
    }
}