#ifndef CATALOGUE_H
#define CATALOGUE_H

#include<row_def.h>

// A catalogue is a set of pages in you database that store what 
// different page_types, tables and columns you have in yout database

// page_type for CATALOGUE page is 0, hence it is reserved
// please use any one of the other 255 remaining page_types for your custom page types
// the first page on the system (i.e. page 0) is reserved to be a catalogue page

// this pages are, non transactional and readonly to the user and only managed by the database engine
// the storage engine will take care to cache them, in further development

const row_def TABLE_CATALOGUE_sch = {
										18,
										2,
										{
											// table unique id
											{
												0,
												2,
												UNSIGNED_INT
											},
											// table name
											{
												2,
												16,
												CHAR_STRING
											},
											// page id corresponding to first page in the table 
											{
												18,
												4,
												UNSIGNED_INT
											}
										}
									};

const row_def COLUMN_CATALOGUE_sch = {
										20,
										3,
										{
											// column unique id
											{
												0,
												2,
												UNSIGNED_INT
											},
											// column name
											{
												2,
												16,
												CHAR_STRING
											},
											// table unique id this column belongs to
											{
												18,
												2,
												CHAR_STRING
											}
										}
									};

const row_def INDEX_CATALOGUE_sch = {
										25,
										5,
										{
											// index unique id
											{
												0,
												2,
												UNSIGNED_INT
											},
											// index name
											{
												2,
												16,
												CHAR_STRING
											},
											// index_type, // hashindex, btree, b+tree, T tree etc (what ever you want to have in your database)
											{
												18,
												1,
												UNSIGNED_INT
											},
											// table unique id this index belongs to
											{
												19,
												2,
												UNSIGNED_INT
											},
											// page id corresponding to first page in the index
											{
												21,
												4,
												UNSIGNED_INT
											}
										}
									};

const row_def INDEX_COLUMN_CATALOGUE_sch = {
										6,
										3,
										{
											// index unique id
											{
												0,
												2,
												UNSIGNED_INT
											},
											// column uniqie id
											{
												2,
												2,
												UNSIGNED_INT
											},
											// this field helps in knowing the index, column relation type
											// to specifi ordering or inclusions of column in the index etc
											{
												4,
												2,
												UNSIGNED_INT
											}
										}
									};


#endif