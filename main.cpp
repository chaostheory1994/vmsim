/* 
 * File:   main.cpp
 * Author: jacob
 *
 * Created on March 17, 2015, 3:54 PM
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#ifdef __linux__
#include <unistd.h>
#endif
#include "PageTable.h"
#define SUCCESS 0
#define FAILURE -1

/*
 * GLOBALS
 */
PageTable* PT;
/* 
 * This method is here to interpret the arguments provided
 * it will return 1 if there is an error 0 if success
 * Error messages will be printed if an error occurs.
 */
void print_help(){
    puts("vmsim -n <numframes> -a <opt|clock|aging|work> [-r <refresh>][-t <tau>] <tracefile>\n");
    puts("-h | --help prints this message");
    puts("-n Sets the number of frames in physical memory.");
    puts("-a Sets which algorithm will be used to determine an eviction.");
    puts("-r The refresh rate for the aging algorithm.");
    puts("-t tau for the Working Set algorithm.");
    puts("Make sure to use a valid trackfile.");
}

/*
 *  This method is here to read through the arguments provided 
 * It will set globals acordingly
 */
int readArgs(int argc, char** argv) {
    // Variables to store the arguments when read.
    bool printHelp;
    int i = 1;
    int alg = -1;
    int frames = -1;
    int param = -1;
    char* filename;
    int filename_size = strlen(argv[argc - 1]); // Get the filename string size.
    if(argc == 1){
        print_help();
        return FAILURE;
    }
    filename = new char[filename_size];
    strcpy(filename, argv[argc - 1]);
    // Lets iterate throught the args to find the rest of them
    for(i = 1; i < argc - 1; i++){
        if(!strcmp(argv[i], "-h")){
            print_help();
            return FAILURE;
        }
        else if(!strcmp(argv[i], "-n")){
            // The means the next argument is the number of frames.
            i++;
            frames = atoi(argv[i]);
        }
        else if(!strcmp(argv[i], "-a")){
            // The next argument is the algorithm wanting to be used.
            i++;
            if(!strcmp(argv[i], "opt")) alg = OPT;
            else if(!strcmp(argv[i], "clock")) alg = CLOCK;
            else if(!strcmp(argv[i], "aging")) alg = AGING;
            else if(!strcmp(argv[i], "working")) alg = WORKING_SET_CLOCK;
            else{
                print_help();
                return FAILURE;
            }
        }
        else if(!strcmp(argv[i], "-r")){
            i++;
            param = atoi(argv[i]);
        }
        else if(!strcmp(argv[i], "-t")){
            i++;
            param = atoi(argv[i]);
        }
    }
    
    // Now lets check if the arguments are valid.
    if(frames == -1 || alg == -1){
        print_help();
        return FAILURE;
    }
    
    if(alg == AGING || alg == WORKING_SET_CLOCK){
        if(param == -1){
            print_help();
            return FAILURE;
        }
    }
    
    PT = new PageTable(frames, alg, filename);
    if(!PT->isFileOpen()){
        puts("Failed to open the file:");
        puts(filename);
#ifdef __linux
        puts("The current working director is:");
        char buff[100];
        getcwd(buff, sizeof(buff));
        puts(buff);
#endif
        delete PT;
        return FAILURE;
    }
    
    if(alg == AGING || alg == WORKING_SET_CLOCK){
        PT->setModifier(param);
    }
    return SUCCESS;
}


/* Main function
 * Will start by reading arguments
 * Then it will initialize the page table
 * Them perform the algorithm specified
 * Finally it will print the results
 */
int main(int argc, char** argv) {
    /* First thing is to read the arguments */
    if(readArgs(argc, argv) == SUCCESS){
        PT->beginFileTraverse();
        PT->printTrace();
        delete PT;
    }
    return 0;
}

