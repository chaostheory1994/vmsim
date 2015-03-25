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
#include <queue>
#include <unistd.h>

/*
 * Constructor
 * 
 * int frame - The number of frames in physical memory
 * int alg   - The algorithm to be used for evicting pages 
 * char** filename - the name of the tracefile to be used 
 */
PageTable::PageTable(int frames, int alg, char* filename) {
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
    tracefile = fopen(filename, "r");
    
#ifdef USEOLDFUTURE
    // Future table for opt initialize
    next_occur = NULL;
#else
    // Initial recording of future.
    if(tracefile != NULL && alg == OPT) find_future_t();
#endif
    
    
    
    // Initialize the stat variables
    page_faults = 0;
    mem_accesses = 0;
    total_writes = 0;
    
    age = 0;
    
    parameter = -1;
    tau = -1;
    
    frames_used = 0; // At the start 0 frames are being used.
    algorithm = alg;
    
}

PageTable::PageTable(const PageTable& orig) {
}

/* Deconstructor*/
PageTable::~PageTable() {
    delete[] pTable;
    delete[] fTable;
#ifdef USEOLDALGORITHM
    if(algorithm == OPT) delete[] next_occur;
#else
#endif
    fclose(tracefile);
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
    // Update Page Table
    frames_used++;
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
    // Update stats
    if(fTable[frame]->isDirty) total_writes++;
    
    // Checks have passed. Proceed to evict frame.
    fTable[frame]->isDirty = false;
    fTable[frame]->frameNum = NO_FRAME;
    fTable[frame] = NULL;
    // Update Page Table
    frames_used--;
    // Update stats
}

/*
 * A simple method that will set the refresh value needed
 * for 2 of the algorithm
 */
void PageTable::setRefresh(int param){
    parameter = param;
}
 /*
  * Another simple method that will set tau for working set.
  */
void PageTable::setTau(int param){
    tau = param;
}


#ifdef USEOLDFUTURE
/*
 * This is a sub method for opt
 * It will look for the first occurance of the page in the future.
 * Then return the number of memory accesses that is.
 * It will also restore the current file pointer.
 */
int PageTable::find_future(int page){
    printf("OPT: Looking for future of page %x\n", page);
    unsigned long position; // To store the position
    unsigned int fpage = 0;
    int retval = mem_accesses;
    char c = 0;
    fflush(tracefile);
    position = ftell(tracefile); // Store position
    // Now that we have where we left off store, we can look into the future.
    do{
        printf("%x is not equal to %x\n", fpage, page);
        // Iterate the return value
        retval++;
        // Read an address
        fscanf(tracefile, "%x %c", &fpage, &c);
        // Convert to page number
        fpage = fpage >> 12;
    }while(fpage != page && !feof(tracefile));
    // Restore place in file
    fseek(tracefile, position, SEEK_SET);
    // Did we find the next position?
    if(fpage != page){
        return -1;
    }
    else return retval;
}
#else
/*
 * This is a second method for speeding up opt.
 * It will parse the file
 * and build a record in the page table.
 */
void PageTable::find_future_t(){
    unsigned int adr;
    char c;
    int thread_location = 0;
    std::cout << "Parsing file and recording future." << std::endl;
    while(!feof(tracefile)){
        thread_location++;
        fscanf(tracefile, "%x %c", &adr, &c); // Read address
        adr = adr >> 12; // convert to page number
        pTable[adr].q.push(thread_location);
    }
    std::cout << "Future Recorded!" << std::endl;
    fseek(tracefile, 0, SEEK_SET);
}
#endif

/* This is an implementation of the optimal algorithm
 * It will assume perfect knowledge by creating a thread that
 * will provide perfect knowledge. This way the main thread
 * will look like a normal algorithm. 
 */
void PageTable::opt(int page){
    int i;
#ifdef USEOLDFUTURE
    // Does the array need initialized?
    if(next_occur == NULL) next_occur = new int[num_frames];
#else
    // Lets pop the pages queue.
    pTable[page].q.pop();
#endif
    
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
#ifdef USEOLDFUTURE
        // Lets find how far in the future this page will be next accessed.
        next_occur[i] = find_future(page);
#endif
    }
    else{
        int valid_evict = 0;
        // The frames are all full.
        // We need to evict one.
#ifdef USEOLDFUTURE
        // We simply look for a -1 or the largest value in next_occur.
        for(i = 1; i < num_frames; i++){
            if(next_occur[i] > next_occur[valid_evict]){
                valid_evict = i;
            }
            else if(next_occur[i] = -1){
                valid_evict = i;
                break;
            }
        }
        // Evict the frame
        evictpage(valid_evict);
        // Place page into frame.
        pagetoframe(page, valid_evict);
        // update next_occur for new page.
        next_occur[valid_evict] = find_future(page);
#else
        valid_evict = -1;
        // This is easy, we search the inverted page table for the largest next occurance.
        // If we find that the value is null, we will sleep the main thread.
        // Unless however, the future thread has finished.
        for(i = 0; i < num_frames; i++){
            // If the queue is empty, then all occurances of that page are done and we can evict.
            if(fTable[i]->q.empty()){
                valid_evict = i;
                break;
            } // If an element is in the queue, we check against the other queue elements.
            else if(valid_evict == -1 || fTable[i]->q.front() > fTable[valid_evict]->q.front()){
                valid_evict = i;
            }
        }
        // Next we evict the page
        evictpage(valid_evict);
        // Next we put the correct page into memory
        pagetoframe(page, valid_evict);
        
#endif
    }
}

/* This is the clcok algorithm 
 * It will use a circular queue to determine the next eviction 
 */
void PageTable::notworking_clock(int page){
    int i;
    int valid_page;
    static int curr = 0; // Clock algorithm index.
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
    }
    else{
        // The frames are all full.
        // We need to evict one.
        // We will simply use the array as the clock.
        bool foundValid = false;
        while(!foundValid){
            if(fTable[curr]->isReferenced){
                // The page had been referenced. Unreferencing.
                fTable[curr]->isReferenced = 0;
            }
            else{
                // The page is a valid choice for replacing.
                valid_page = curr;
                foundValid = true;
            }
            // Advance the clock
            curr++;
            // Do we need to cycle around?
            if(curr == num_frames) curr = 0; 
        }
        // Now that we found the page to evict, lets do that.
        evictpage(valid_page);
        pagetoframe(page, valid_page);
    }
    
}

/* This is the aging algorithm
 * It will approximate LRU with an 8-bit counter
 * Every memory access is a shift right.
 */
void PageTable::aging(int page){
    int i;
    // Is the parameter set - refresh?
    if(parameter == -1) return;
    int valid_evict = -1;
    // First thing we need to do is iterate the time for everyone if refresh is up.
    if((mem_accesses - age) >= parameter){
        for(i = 0; i < num_frames; i++){
            if(fTable[i] != NULL){ // If a page exists in the frame. shift right the reference bit.
                fTable[i]->isReferenced = fTable[i]->isReferenced >> 1;
            }
        }
        age = mem_accesses;
    }
    // Next thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
        // Next algorithm specific add a bit to the end.
        fTable[i]->isReferenced |= REF_END_BIT;
        
    }
    else{
        // The frames are all full.
        // We need to evict one.
        // We need to iterate through and find which frame to evict
        for(i = 0; i < num_frames; i++){
            if(valid_evict == -1 || fTable[i]->isReferenced < fTable[valid_evict]->isReferenced)
                valid_evict = i;
        }
        evictpage(valid_evict); // Evict that page
        pagetoframe(page, valid_evict); // Put that page into a frame.
        // Now page to frame will set isReferenced to one.
        // This will not hurt anything since it will be shifted off anyway.
        // However, we do need to still add on the end bit.
        fTable[valid_evict]->isReferenced |= REF_END_BIT;
        
    }
    
}

/* The Working Set Clock algorithm
 * This will have similar workings to the clock aglorithm
 * however, will make use of the timestamp.
 * The project page did not tell us how to deal with a cycle.
 * So I have it set to evict the page with the largest age.
 */
void PageTable::working_clock(int page){
    int i;
    static int curr = 0; // The clock hand.
    int valid_page; // The stored valid page.
    int no_choice = -1; // In case of a cycle.
    // This method required a modifier to be set.
    if(parameter == -1 || tau == -1) return;
    // Update reference from fresh
    if((mem_accesses - age) >= parameter){
        for(i = 0; i < num_frames; i++){
            if(fTable[i] != NULL){ // If a page exists in the frame. set reference bit to 0
                fTable[i]->isReferenced = 0;
            }
        }
        age = mem_accesses;
    }
    // First thing to do is check to see if there is a frame available.
    if(frames_used < num_frames){
        // There is a frame available. Lets find it and put it in.
        for(i = 0; fTable[i] != NULL; i++);
        pagetoframe(page, i); // Insert the page into that frame.
    }
    else{
        // The frames are all full.
        // We need to evict one.
        int hasCycled = curr;
        bool foundValid = false;
        do{
            // Is the reference bit set?
            if(fTable[curr]->isReferenced == 1){
                // The page was in use, we should not evict.
                // We should update properties.
                fTable[curr]->isReferenced = 0;
                fTable[curr]->timeStamp = mem_accesses;
            }
            else{
                // The reference bit was not set.
                // Is the age within tau to evict?
                if((mem_accesses - fTable[curr]->timeStamp) > tau){
                    // This means the age is outside the working set.
                    // However we need to check if it is dirty.
                    if(fTable[curr]->isDirty){
                        // Undirty it
                        fTable[curr]->isDirty = false;
                        // The page was dirty. check if it was the worst age
                        if(no_choice == -1 ||
                                fTable[curr]->timeStamp < fTable[no_choice]->timeStamp){
                            // This was the oldest age.
                            no_choice = curr;
                        }
                    }
                    else{
                        // The page was clean so we can easily replace it
                        valid_page = curr;
                        foundValid = true;
                    }
                }
                else{
                    // The page is not old enough and considered the working set.
                    // However, we are going to see which is the oldest incase all pages
                    // seem to be in the working set.
                    if(no_choice == -1 ||
                            fTable[curr]->timeStamp < fTable[no_choice]->timeStamp){
                        // This was the oldest age.
                        no_choice = curr;
                    }
                }
            }
            // Next we need to iterate curr;
            curr++;
            if(curr >= num_frames) curr = 0;
        } while((hasCycled != curr && !foundValid) || no_choice == -1);
        
        // Did we encounter a cycle?
        if(!foundValid){
            valid_page = no_choice;
        }
        
        // Now that we have a page to evict, lets do so.
        evictpage(valid_page);
        // Put new page in.
        pagetoframe(page, valid_page);
        
    }
    
}

/* 
 * This method will convert a given page address to a page number
 * Then use the page table to determine if it is in a frame 
 * Will put it in a frame and update some of the specified bits 
 */
void PageTable::useAddress(unsigned int adr, bool isWriting){
    int i;
    static unsigned int prev =  0;
    // Lets convert the address to a page number.
    unsigned int page_num = adr & PAGE_ADDRESS_AND; // And off the offset
    page_num = page_num >> 12; // Shift the page number to be correct
    
    // Then lets iterate the a stat.
    mem_accesses++;

    prev = adr;
    // Iterate Writes if necessary
    if(isWriting) total_writes++;
    
    // Is the page already in a frame?
    if(pTable[page_num].frameNum == NO_FRAME){
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
    } 
    else{
        // This is for aging algorithm as to iterate all the reference bits.
        if(algorithm == AGING && (mem_accesses - age) >= parameter){
            for(i = 0; i < num_frames; i++){
                if(fTable[i] != NULL) // If a page exists in the frame. shift right the reference bit.
                    fTable[i]->isReferenced = fTable[i]->isReferenced >> 1;
            }
            pTable[page_num].isReferenced |= REF_END_BIT;
            // Update age
            age = mem_accesses;
        }
        else if(algorithm == AGING){
            pTable[page_num].isReferenced |= REF_END_BIT;
        }
        // This is for OPT algorithm
        else if(algorithm == OPT){
#ifdef USEOLDFUTURE
            // if a page in a frame hits its next occurance.
            // Then we need to update the next occurance to be even further. 
            next_occur[pTable[page_num].frameNum] = find_future(page_num);
#else
            // If a hit occurs, we simply deque it from the queue.
            pTable[page_num].q.pop();
#endif
        }
        else{
            if((mem_accesses - age) >= parameter){
                for(i = 0; i < num_frames; i++){
                    if(fTable[i] != NULL){ // If a page exists in the frame. set reference bit to 0;
                        fTable[i]->isReferenced = 0;
                    }
                }
                age = mem_accesses;
            }
            pTable[page_num].isReferenced = 1;
        }
    }
    
}

/*
 * This method starts the main loop to begin reading in addresses
 * It will read an address and call useAddress
 */
void PageTable::beginFileTraverse(){
    unsigned int adr;
    char mode;
    bool foundEOF = false;
    while(!foundEOF){
        fscanf(tracefile, "%x %c", &adr, &mode);
        /*std::cout.setf(std::ios::hex, std::ios::basefield);
        std::cout << "Read: " << adr;
        std::cout.unsetf(std::ios::hex);
        std::cout << " " << mode << std::endl;*/
        if(!feof(tracefile))useAddress(adr, (mode == 'W' || mode == 'w'));
        else foundEOF = true;
    }
}

/* 
 * A simple print method to print the current algorithm in use.
 */
void PageTable::printTrace(){
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
    std::cout << "Number of Frames: " << num_frames << std::endl;
    std::cout << "Total Memory Accesses: " << mem_accesses << std::endl;
    std::cout << "Total Page Faults: " << page_faults << std::endl;
    std::cout << "Total Writes to Disk: " << total_writes << std::endl;
}

bool PageTable::isFileOpen(){
    return (tracefile != NULL);
}