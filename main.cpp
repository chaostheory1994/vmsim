/* 
 * File:   main.cpp
 * Author: jacob
 *
 * Created on March 17, 2015, 3:54 PM
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "PageTable.h"
#define SUCCESS 0

/*
 * GLOBALS
 */
int algorithm;

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
    puts("-r IDK yet.");
    puts("-t IDK yet.");
    puts("Make sure to use a valid trackfile.");
}

/*
 *  This method is here to read through the arguments provided 
 * It will set globals acordingly
 */
int readArgs(int argc, char** argv) {
    // First lets setup a variable for the help command
    
    return SUCCESS;
}

/* 
 * A method to initialize all the variables and the page table. 
 */
void init(){
    
    
}

/* This method is to print the trace results as according to the globals
 */
void print_trace(){
    
}

/* Main function
 * Will start by reading arguments
 * Then it will initialize the page table
 * Them perform the algorithm specified
 * Finally it will print the results
 */
int main(int argc, char** argv) {
    /* First thing is to read the arguments */
    //if(readArgs(argc, argv))
    //    return 0;
    PageTable p(5, OPT, 0, "");
    p.printAlgorithm();
    p.useAddress(0xffffffff, false);
    return (EXIT_SUCCESS);
}

