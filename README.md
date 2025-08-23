# TupleStore

This library provides you with a very simple 
 * **primarily row oriented - slotted-page & fixed-array-page storage models,**
 * **for fixed or variable length runtime-defined tuples with a runtime type system,**
 * **to be stored on fixed sized pages for block storage systems.**

Started as a project to be used for managing page layout in a database storage engine. It outgrew into a data format for runtime-defined type system. It supports variery of primitve types and also nested arrays and tuples.

To reiterate : This project only provides primitive utility functions/structures to systematically organize your data in tuples of runtime-defined datatypes in a specific (slotted page or fixed array page) model for a block-storage-memory consisting of pages of predefined fixed size.

Note:: The TupleStore API deals with only 32 bit unsigned integers for most cases (sizes, offsets and indices). This imposes a hard limit of 4GB on the page size. But remember uint32_t can overflow, and adding overflow checks in TupleStore for every mathematical operation is a performance killer.

Project Limits :
 PAGE_LAYOUT LIMITS :
  * page_size :                   [128, 2GB)
  * page_header_size :            [0, page_size-32]
  * max_size (for a tuple/data) : [1, 2GB) (no data_type_info can have max_size set more than 2GB)

Overflow prone functions :
  * get_minimum_page_size() -> limit tuple_count that you pass as parameter
  * finalize_tuple_def() -> limit max size of element and maximum number of elements, so as the number does not cross 2GB for tuples and arrays
  * expand_element_count_for_element_in_tuple() and can_expand_element_count_for_element_in_tuple() -> accumulate all the elements you want to insert in memory and ensure that their (sum of sizes + 4 * count) does not cross 2GB and and only then attempt to expand the container, else their may be overflow

## Setup instructions
**Install dependencies :**
 * [Cutlery](https://github.com/RohanVDvivedi/Cutlery)
 * [SerializableInteger](https://github.com/RohanVDvivedi/SerializableInteger)

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/TupleStore.git`

**Build from source :**
 * `cd TupleStore`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-ltuplestore -lcutlery` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<tuplestore/tuple_def.h>`
   * `#include<tuplestore/tuple.h>`
   * `#include<tuplestore/page_layout.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd TupleStore`
 * `sudo make uninstall`
