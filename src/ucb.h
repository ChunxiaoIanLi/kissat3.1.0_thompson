#ifndef _ucb_h_INCLUDED
#define _ucb_h_INCLUDED

#include <stdlib.h>

struct record {
    unsigned int bandit;
    double reward;
};

struct window {
    struct record *records; // Pointer to the array of records
    int size;               // Size of the window
    int index;              // Current position for insertion
    int counts;            // Number of records ever inserted
};

void init_window(struct window *w, int window_sz);

void free_window(struct window *w);

void add_record(struct window *w, struct record newRecord);

#endif