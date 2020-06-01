#ifndef CELL_OPERATIONS_H
#define CELL_OPERATIONS_H

#include<cell_definition.h>

int compare_cell_allowed(cell_definition* cell_def_1, cell_definition* cell_def_2);

// returns 1 if cell1 > cell2, returns -1 if cell2 < cell1, else returns 0 if they are equal
// returns -2 if they can not be compared
int compare_cell(void* cell_pos_1, cell_definition* cell_def_1, void* cell_pos_2, cell_definition* cell_def_2);

#endif