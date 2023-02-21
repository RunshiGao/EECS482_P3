#include "vm_pager.h"
#include <queue>
#include <unordered_map>
#include <string>
#include <cstring>
#include <iostream>
#include "page_structure.h"

using namespace std;

// key is the pid of process
unordered_map<int, process_mem*> pid_mem;
// the pointer of the porocess memory structure that is running
process_mem* current_process;
// the element is the index of free physical memory
queue<int> free_phys_mem;
// the element is the index of free swap block
queue<int> free_swap_block;
// the element is the index of physical memory
queue<int> clock_queue;
// the first key is index of physical memory
// the second key is the pid of process
// the vlaue of the inner map is the index of virtual memory
unordered_map<int, unordered_map<int, int>> pp2vp_swap;
// the first key is index of swap block
// the second key is the pid of process
// the vlaue of the inner map is the index of virtual memory
unordered_map<int, unordered_map<int, int>> block2vp_swap;

unordered_map<int, unordered_map<int, vector<int>>> pp2vp_file;

unordered_map<string, unordered_map<int, unordered_map<int, vector<int>>>> block2vp_file;

// use for evict
//physical page-><<filename,block>,<reference,dirty>>
unordered_map<int, pair<pair<string, int>, pair<bool, bool>>> ready2evict_file;
// use for vm_map
unordered_map<string, unordered_map<int, int>> block2pp_file;

// it is for eager swap reservation
unsigned int avail_swap_block;

void vm_init(unsigned int memory_pages, unsigned int swap_blocks) {
    // make a zero page
    memset(vm_physmem, 0, VM_PAGESIZE);
    for(unsigned int i = 1; i < memory_pages; i++) {
        free_phys_mem.push(i);
    }

    for(unsigned int i=0; i < swap_blocks; i++) {
        free_swap_block.push(i);
    }

    avail_swap_block = swap_blocks;
}

void vm_switch(pid_t pid) {
    page_table_base_register = &pid_mem[pid]->hw_page_table;
    current_process = pid_mem[pid];
}

int vm_create(pid_t parent_pid, pid_t child_pid) {
    if(pid_mem.find(parent_pid) == pid_mem.end()) {
        // parent process is not being managed by the pager, the arena is empty
        process_mem* child_mem = new process_mem();
        child_mem->pid = child_pid;
        child_mem->num_swap_based = 0;
        pid_mem[child_pid] = child_mem;
    } else {
        // the child has a parent
        process_mem* parent_mem = pid_mem[parent_pid];
        if (avail_swap_block < parent_mem->num_swap_based) {
            // not enough space for swap block
            return -1;
        } else {
            // create page table for child
            avail_swap_block -= parent_mem->num_swap_based;
            process_mem* child_mem = new process_mem(*parent_mem);
            child_mem->num_swap_based = parent_mem->num_swap_based;
            child_mem->pid = child_pid;
            pid_mem[child_pid] = child_mem;
            // update the hash_map (change reference)
            for (unsigned int vp_index = 0; vp_index < parent_mem->available_vm_index; vp_index++) {
                if (parent_mem->os_page_table[vp_index].type == 0) {
                    if (parent_mem->os_page_table[vp_index].resident) {
                        int pp_index = parent_mem->hw_page_table.ptes[vp_index].ppage;
                        if (pp_index != 0) {  
                            pp2vp_swap[pp_index][child_pid] = vp_index;
                            // write protect all swap based pages
                            for (auto m : pp2vp_swap[pp_index]) {
                                process_mem* p = pid_mem[m.first];
                                int vp_index = m.second;
                                p->hw_page_table.ptes[vp_index].write_enable = 0;
                            }
                        }                            
                    } else {
                        block2vp_swap[parent_mem->os_page_table[vp_index].block][child_pid] = vp_index;
                    }
                } else {
                    string filename = parent_mem->os_page_table[vp_index].filename;
                    int block = parent_mem->os_page_table[vp_index].block;
                    block2vp_file[filename][block][child_pid].push_back(vp_index);
                    if (parent_mem->os_page_table[vp_index].resident) {
                        int pp_index = parent_mem->hw_page_table.ptes[vp_index].ppage;
                        pp2vp_file[pp_index][child_pid].push_back(vp_index);
                    }
                }
            }
        }
    }
    return 0;
}

void *vm_map(const char *filename, unsigned int block) {
    if(filename == nullptr) {
        // it is the swap based virtual memory
        if(current_process->available_vm_index < VM_ARENA_SIZE/VM_PAGESIZE && avail_swap_block != 0) {
            // has not be referenced
            avail_swap_block--;
            current_process->num_swap_based++;
            current_process->os_page_table[current_process->available_vm_index].valid = true;
            current_process->os_page_table[current_process->available_vm_index].resident = true;
            current_process->os_page_table[current_process->available_vm_index].type = 0;
            current_process->hw_page_table.ptes[current_process->available_vm_index].ppage = 0;
            current_process->hw_page_table.ptes[current_process->available_vm_index].read_enable = 1;
            current_process->hw_page_table.ptes[current_process->available_vm_index].write_enable = 0;
            current_process->available_vm_index++;
        } else {
            return nullptr;
        }
    } else {
        // it is file based virtual memory
        if(current_process->available_vm_index < VM_ARENA_SIZE/VM_PAGESIZE) {
            if((char*)filename >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || (char*) filename < (char*) VM_ARENA_BASEADDR) {
                return nullptr;
            }
            auto res = current_process->convert(filename);
            if(res.second == -1) {
                return nullptr;
            }
            string filename_str = res.first;
            current_process->os_page_table[current_process->available_vm_index].valid = true;
            current_process->os_page_table[current_process->available_vm_index].resident = false;
            current_process->os_page_table[current_process->available_vm_index].type = 1;
            current_process->os_page_table[current_process->available_vm_index].filename = filename_str;
            current_process->os_page_table[current_process->available_vm_index].block = block;
            block2vp_file[filename_str][block][current_process->pid].push_back(current_process->available_vm_index);
            if (block2pp_file[filename_str].find(block) != block2pp_file[filename_str].end()) {
                // if exist
                current_process->os_page_table[current_process->available_vm_index].resident = true;
                int ppage = block2pp_file[filename_str][block];
                current_process->hw_page_table.ptes[current_process->available_vm_index].ppage = ppage;
                if(pp2vp_file[ppage].size() != 0) {
                    auto it = pp2vp_file[ppage].begin();
                    int pid = it->first;
                    int vp_index = it->second[0];
                    process_mem* p = pid_mem[pid];
                    current_process->os_page_table[current_process->available_vm_index] = p->os_page_table[vp_index];
                    current_process->hw_page_table.ptes[current_process->available_vm_index] = p->hw_page_table.ptes[vp_index];
                    pp2vp_file[ppage][current_process->pid].push_back(current_process->available_vm_index);
                } else if(ready2evict_file.find(ppage) != ready2evict_file.end()) {
                    if (!ready2evict_file[ppage].second.first) {
                        current_process->hw_page_table.ptes[current_process->available_vm_index].write_enable = 0;
                        current_process->hw_page_table.ptes[current_process->available_vm_index].read_enable = 0;
                    } else {
                        if (ready2evict_file[ppage].second.second) {
                            current_process->hw_page_table.ptes[current_process->available_vm_index].write_enable = 1;
                            current_process->hw_page_table.ptes[current_process->available_vm_index].read_enable = 1;
                        } else {
                            current_process->hw_page_table.ptes[current_process->available_vm_index].write_enable = 0;
                            current_process->hw_page_table.ptes[current_process->available_vm_index].read_enable = 1;
                        }
                    }
                    pp2vp_file[ppage][current_process->pid].push_back(current_process->available_vm_index);
                    current_process->os_page_table[current_process->available_vm_index].dirty = ready2evict_file[ppage].second.second;
                    current_process->os_page_table[current_process->available_vm_index].referenced = ready2evict_file[ppage].second.first;
                    ready2evict_file.erase(ppage);
                }
            }
            current_process->available_vm_index++;
        } else {
            return nullptr;
        }
    }
    return (void*)((char*)VM_ARENA_BASEADDR + (current_process->available_vm_index - 1) * VM_PAGESIZE);
}


int handle_swap(const void* addr, bool write_flag, unsigned int virtual_index) {
    // if nonresident, we need to read from the disk
    if (current_process->os_page_table[virtual_index].resident == false) {
        int ppage = get_ppage();
        if (ppage == -1) {
            return -1;
        }
        clock_queue.push(ppage);
        int block = current_process->os_page_table[virtual_index].block;
        pp2vp_swap[ppage] = block2vp_swap[block];
        block2vp_swap.erase(block);
        int flag = file_read(nullptr, block, (char*)vm_physmem + ppage * VM_PAGESIZE);
        if (flag == -1) {
            return -1;
        }
        read_page(ppage);
    }
    // whether we need to set the dirty bit and copy write
    if (write_flag) {
        // if it is initial assign to a 0 page, assign a new page
        // if it is shared by other process, assign a new page
        int ppage = current_process->hw_page_table.ptes[virtual_index].ppage;
        // if this condition is true, then it's copy-on-write
        if(pp2vp_swap[ppage].size() > 1 || ppage == 0) {
            // copy on write
            //char buffer[VM_PAGESIZE];
            char* buffer = (char*)malloc(VM_PAGESIZE);
            memcpy(buffer, (char*)vm_physmem + ppage * VM_PAGESIZE, VM_PAGESIZE);
            // need to update virtual page variables
            read_page(ppage);
            if(ppage != 0) {
                pp2vp_swap[ppage].erase(current_process->pid);
                // very very important
                if(pp2vp_swap[ppage].size() == 1) {
                    auto it = pp2vp_swap[ppage].begin();
                    process_mem* p = pid_mem[it->first];
                    int vp_index = it->second;
                    p->set_hw(vp_index);
                }
            }
            int new_ppage = get_ppage();
            if (new_ppage == -1) {
                return -1;
            }
            clock_queue.push(new_ppage);
            pp2vp_swap[new_ppage][current_process->pid] = virtual_index;
            memcpy((char*)vm_physmem + new_ppage * VM_PAGESIZE, buffer, VM_PAGESIZE);
            current_process->os_page_table[virtual_index].dirty = true;
            current_process->os_page_table[virtual_index].referenced = true;
            current_process->hw_page_table.ptes[virtual_index].ppage = new_ppage;
            int free_block = free_swap_block.front();
            free_swap_block.pop();
            current_process->os_page_table[virtual_index].block = free_block;
            current_process->set_hw(virtual_index);
            free(buffer);
        } else {
            current_process->os_page_table[virtual_index].dirty = true;
            current_process->os_page_table[virtual_index].referenced = true;
            current_process->set_hw(virtual_index);
        }
    } else {
        read_page(current_process->hw_page_table.ptes[virtual_index].ppage);
    }
    return 0;
}

int handle_file(const void* addr, bool write_flag, unsigned int virtual_index) {
    // if nonresident, we need to read from the disk

    if (current_process->os_page_table[virtual_index].resident == false) {
        int ppage = get_ppage();
        if (ppage == -1) {
            return -1;
        }
        string filename = current_process->os_page_table[virtual_index].filename;
        int block = current_process->os_page_table[virtual_index].block;
        int flag = file_read(filename.c_str(), block, (char*)vm_physmem + ppage * VM_PAGESIZE);
        if (flag == -1) {
            free_phys_mem.push(ppage);
            return -1;
        }
        clock_queue.push(ppage);
        pp2vp_file[ppage] = block2vp_file[filename][block];
        block2pp_file[filename][block] = ppage;
        read_page_file(ppage);
    }
    int pp_index = current_process->hw_page_table.ptes[virtual_index].ppage;
    if (write_flag) {
        write_page_file(pp_index);
    } else {
        read_page_file(pp_index);
    }
    return 0;
}

int vm_fault(const void* addr, bool write_flag) {
    if((char*)addr >= ((char*)VM_ARENA_BASEADDR + VM_ARENA_SIZE) || (char*) addr < (char*) VM_ARENA_BASEADDR) {
            return -1;
    }
    // the index is the virtual page index of arena
    unsigned int virtual_index = ((uintptr_t)addr-(uintptr_t)VM_ARENA_BASEADDR) / VM_PAGESIZE;
    if(virtual_index >= current_process->available_vm_index) {
        return -1;
    }
    // in the case that swap-based
    if (current_process->os_page_table[virtual_index].type == 0) {
        return handle_swap(addr, write_flag, virtual_index);
    } else {
        // file-based
        return handle_file(addr, write_flag, virtual_index);       
    }
}

void vm_destroy() {
    // remove reference, including pp2vp and block2vp
    for (unsigned int vp_index = 0; vp_index < current_process->available_vm_index; vp_index++) {
        if (current_process->os_page_table[vp_index].type == 0) {
            avail_swap_block++;
            if (current_process->os_page_table[vp_index].resident) {
                int ppage = current_process->hw_page_table.ptes[vp_index].ppage;
                if(ppage != 0) {
                    pp2vp_swap[ppage].erase(current_process->pid);
                    // if size of pp2vp[ppage] is 0, free physical page and free assigned block
                    if (pp2vp_swap[ppage].empty()) {
                        free_swap_block.push(current_process->os_page_table[vp_index].block);
                    }
                    // if there is only one process, possible to set hw
                    if (pp2vp_swap[ppage].size() == 1) {
                        int pid = pp2vp_swap[ppage].begin()->first;
                        int vp_index = pp2vp_swap[ppage].begin()->second;
                        process_mem* ptr = pid_mem[pid];
                        ptr->set_hw(vp_index);
                    }
                } 
            } else {
                block2vp_swap[current_process->os_page_table[vp_index].block].erase(current_process->pid);
                // if size of block2vp[block] is 0, free block
                if (block2vp_swap[current_process->os_page_table[vp_index].block].empty()) {
                    free_swap_block.push(current_process->os_page_table[vp_index].block);
                }
            }
        } else {
            // if it is file based
            // first, take care of phys mem
            string filename = current_process->os_page_table[vp_index].filename;
            int block = current_process->os_page_table[vp_index].block;
            if (current_process->os_page_table[vp_index].resident) {
                int ppage = current_process->hw_page_table.ptes[vp_index].ppage;
                pp2vp_file[ppage].erase(current_process->pid);
                if (pp2vp_file[ppage].size() == 0) {
                    bool dirty = current_process->os_page_table[vp_index].dirty;
                    bool referenced = current_process->os_page_table[vp_index].referenced;
                    ready2evict_file[ppage] = {{filename,block}, {referenced,dirty}};
                }
            }
            // regardless of resident or nonresident, take care of block
            block2vp_file[filename][block].erase(current_process->pid);
        }
    }
    int len = clock_queue.size();
    for (int i = 0; i < len; i++) {
        int pp_index = clock_queue.front();
        clock_queue.pop();
        if (ready2evict_file.find(pp_index) == ready2evict_file.end() && pp2vp_file[pp_index].size() == 0) {
            if (pp2vp_swap[pp_index].size() == 0) {
                free_phys_mem.push(pp_index);
            } else {
                clock_queue.push(pp_index);
            }
        } else {
            clock_queue.push(pp_index);
        }
    }
    delete current_process;
    current_process = nullptr;
}
