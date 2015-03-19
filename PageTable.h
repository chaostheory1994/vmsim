/* 
 * File:   PageTable.h
 * Author: jacob
 *
 * Created on March 18, 2015, 11:19 AM
 */

#ifndef PAGETABLE_H
#define	PAGETABLE_H
#include <cstdlib>
#include <cstdio>
#define PAGE_SIZE 4096
#define PAGE_ADDRESS_AND 0xFFFFF000
#define OPT 0
#define CLOCK 1
#define AGING 2
#define WORKING_SET_CLOCK 3

typedef struct TableEntry{
    bool isDirty; // Dirty Bit
    int isReferenced; // Reference bit. Set as int for aging algorithm
    int frameNum; // Frame Number
    int timeStamp; // Timestamp for certain algorithms.
} TableEntry;

class PageTable {
public:
    PageTable(int, int, int, char*);
    PageTable(const PageTable& orig);
    virtual ~PageTable();
    int getPageFaults();
    void useAddress(unsigned int, bool);
    void printAlgorithm();
    void beginFileTraverse();
    void setModifier(int);
private:
    void future_thread();
    void opt(int);
    void notworking_clock(int);
    void aging(int);
    void working_clock(int);
    void pagetoframe(int, int);
    void evictpage(int);
    int page_faults; // Stat variable
    FILE* tracefile; // File pointer for reading.
    int mem_accesses; // Stat variable
    int total_writes; // Stat variable
    int num_frames; // Number of physical memory frames.
    int num_pages; // Number of pages in virtual memory.
    int algorithm; // The algorithm set to be used.
    int next_to_evict; // This is for the opt algorithm.
    int frames_used; // Used for the start as to see how many frames are in use.
    int parameter; // Used for Working Set/Aging. It is the extra parameter.
    TableEntry* pTable; // The page table itself.
    TableEntry** fTable; // The inverted Page Table.
};

#endif	/* PAGETABLE_H */

