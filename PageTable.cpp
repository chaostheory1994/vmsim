/* 
 * File:   PageTable.cpp
 * Author: jacob
 * 
 * Created on March 18, 2015, 11:19 AM
 */

#include "PageTable.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

/*
 * Constructor
 * 
 * int frame - The number of frames in physical memory
 * int alg   - The algorithm to be used for evicting pages 
 * char** filename - the name of the tracefile to be used 
 */
PageTable::PageTable(int frames, int alg, int ref, char* filename) {
    // Lets first setup the physical memory.
    num_frames = frames;
    fTable = new TableEntry*[num_frames];
    // Now lets set each frame to NULL
    for(int i = 0; i < num_frames; i++) fTable[i] = NULL;
    
    // Next lets initialize the actual page table.
    // First lets see how many pages there are.
    num_pages = PAGE_ADDRESS_AND >> 12;
    pTable = new TableEntry[num_pages];
    
    // Initialize all the pages.
    for(int i = 0; i < num_pages; i++){
        pTable[i].frameNum = -1;
        pTable[i].isDirty = false;
        pTable[i].isReferenced = 0;
        pTable[i].timeStamp = -1;
    }
    
    // We are gonna want to open the file.
    //tracefile = fopen(filename);
    
    // Initialize the stat variables
    page_faults = 0;
    mem_accesses = 0;
    total_writes = 0;
    
    parameter = -1;
    
    frames_used = 0; // At the start 0 frames are being used.
    algorithm = alg;
}

PageTable::PageTable(const PageTable& orig) {
}

/* Deconstructor*/
PageTable::~PageTable() {
    delete[] pTable;
    delete[] fTable;
    //fclose();
}

/* 
 * This method will will simply put a page into a frame
 * It will update the things in the page struct 
 */
void PageTable::pagetoframe(int page, int frame){
    // Checks for bad things.
    if(frame >= num_frames || frame < 0){
        std::cout << "ERR: Frame Number out of bounds." << std::endl;
        return;
    }
    if(page >= num_pages || page < 0){
        std::cout << "ERR: Page Number out of bounds." << std::endl;
        return;
    }
    if(fTable[frame] != NULL){
        std::cout << "ERR: The frame has not been evicted" << std::endl;
        return;
    }
    
    // Now that all the checks passed, we can safely put a page into the frame.
    fTable[frame] = pTable + page;
    pTable[page].isReferenced = 1;
    pTable[page].frameNum = frame;
    pTable[page].timeStamp = mem_accesses;
}

/*
 * This method is used to evict a page from the specified frame.
 * There is no need to fix isReferenced or timeStamp 
 * Frame number needs to be reset for checking if it is in a frame.
 */
void PageTable::evictpage(int frame){
    // First check if frame is valid
    if(frame >= num_frames || frame < 0){
        std::cout << "ERR: The frame number is out of bounds" << std::endl;
        return;
    }
    if(fTable[frame] == NULL){
        std::cout << "ERR: Tried to evict frame already empty" << std::endl;
        return;
    }
    
    // Checks have passed. Proceed to evict frame.
    fTable[frame]->isDirty = false;
    fTable[frame]->frameNum = -1;
    fTable[frame] = NULL;
}

/*
 * A simple method that will set the extra value needed
 * for 2 of the algorithm
 */
void PageTable::setModifier(int param){
    parameter = param;
}

/* 
 * A thread for the optimal function 
 * It is meant to just look to the future while
 * the main thread looks at what the future produced 
 * This will only run when the frames are all filled up.
 */
void PageTable::future_thread(){
    
}

/* This is an implementation of the optimal algorithm
 * It will assume perfect knowledge by creating a thread that
 * will provide perfect knowledge. This way the main thread
 * will look like a normal algorithm. 
 */
void PageTable::opt(int page){
    int i;
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
        frames_used++;
    }
    else{
        // The frames are all full.
        // We need to evict one.
    }
}

/* This is the clcok algorithm 
 * It will use a circular queue to determine the next eviction */
void PageTable::notworking_clock(int page){
    int i;
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
    }
    else{
        // The frames are all full.
        // We need to evict one.
    }
    
}

/* This is the againg algorithm
 * It will approximate LRU with an 8-bit counter
 */
void PageTable::aging(int page){
    int i;
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
    }
    else{
        // The frames are all full.
        // We need to evict one.
    }
    
}

/* The Working Set Clock algorithm
 * 
 * 
 */
void PageTable::working_clock(int page){
    int i;
    // This method required a modifier to be set.
    
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
    }
    else{
        // The frames are all full.
        // We need to evict one.
        
    }
    
}

/* 
 * This method will convert a given page address to a page number
 * Then use the page table to determine if it is in a frame 
 * Will put it in a frame and update some of the specified bits 
 */
void PageTable::useAddress(unsigned int adr, bool isWriting){
    // Lets convert the address to a page number.
    unsigned int page_num = adr & PAGE_ADDRESS_AND; // And off the offset
    page_num = page_num >> 12; // Shift the page number to be correct
    
    // Then lets iterate the a stat.
    mem_accesses++;
    // Iterate Writes if necessary
    if(isWriting) total_writes++;
    
    // Is the page already in a frame?
    if(pTable[page_num].frameNum == -1){
        // Page Fault
        // iterate stat
        page_faults++;
        // Lets send the page number to the correct algorithm.
        switch(algorithm){
            case OPT:
                opt(page_num);
                break;
            case CLOCK:
                notworking_clock(page_num);
                break;
            case AGING:
                aging(page_num);
                break;
            case WORKING_SET_CLOCK:
                working_clock(page_num);
                break;
            default:
                std::cout << "Algorithm set incorrectly" << std::endl;
        }
    } // else no fault occurred so no need to evict anything.
    
}

/*
 * This method starts the main loop to begin reading in addresses
 * It will read an address and call useAddress
 */
void PageTable::beginFileTraverse(){
    
}

/* 
 * A simple print method to print the current algorithm in use.
 */
void PageTable::printAlgorithm(){
    switch(algorithm){
        case OPT:
            std::cout << "Opt Algorithm" << std::endl;
            break;
        case CLOCK:
            std::cout << "Clock Algorithm" << std::endl;
            break;
        case AGING:
            std::cout << "Aging Algorithm" << std::endl;
            break;
        case WORKING_SET_CLOCK:
            std::cout << "Working Set Clock Algorithm" << std::endl;
            break;
        default:
            std::cout << "Something went wrong here." << std::endl;
            break;
    }
}