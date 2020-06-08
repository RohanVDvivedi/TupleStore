#ifndef PAGE_MODIFIERS_H
#define PAGE_MODIFIERS_H

#include<data_access_methods.h>

#include<row_def.h>

// initializes the first pages, and sets everything up
void create_database(const data_access_methods* dam, char* database_name);

// adds a table, to the database, adds columns to database
void create_table(const data_access_methods* dam, char* table_name, row_def* columns);

// adds a table, to the database, adds columns to database
void create_index(const data_access_methods* dam, char* index_name, char* table_name, row_def* columns);

// retuns a new page to read/write to
void* get_new_free_page(const data_access_methods* dam);

void drop_table(const data_access_methods* dam, char* table_name);

#endif