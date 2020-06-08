#ifndef PAGE_MODIFIERS_H
#define PAGE_MODIFIERS_H

#include<string.h>

#include<data_access_methods.h>
#include<file_hdr.h>
#include<page_hdr.h>
#include<row_def.h>

// initializes the first pages, and sets everything up
// returns 0, if a database already exists
int create_or_open_database(const data_access_methods* dam, char* database_name);

// adds a table, to the database, adds columns to database
void create_table(const data_access_methods* dam, char* table_name, row_def* columns);

void drop_table(const data_access_methods* dam, char* table_name);

int close_database(const data_access_methods* dam);

#endif