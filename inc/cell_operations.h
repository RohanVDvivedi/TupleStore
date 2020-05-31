#ifndef CELL_OPERATIONS_H
#define CELL_OPERATIONS_H

#include<cell_definition.h>

int compare_cell_allowed(cell_definition* cell_def_1, cell_definition* cell_def_2);

int compare_cell_distinct(void* cell_pos_1, cell_definition* cell_def_1, void* cell_pos_2, cell_definition* cell_def_2);

#endif