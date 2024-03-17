#include "ucb.h"

#include <stdlib.h>

void init_window(struct window *w, int window_sz) {
    w->records = (struct record *)malloc(window_sz * sizeof(struct record));
    w->size = window_sz;
    w->index = 0;
}

void free_window(struct window *w) {
    free(w->records);
}

void add_record(struct window *w, struct record newRecord) {
    w->records[w->index] = newRecord;
    w->index = (w->index + 1) % w->size; // Circular increment
    w->counts++;
}
