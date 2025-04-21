# TupleStore

This library provides you with a very simple 
 * **primarily row oriented - slotted-page & fixed-array-page storage models,**
 * **for fixed or variable length runtime-defined tuples with a runtime type system,**
 * **to be stored on fixed sized pages for block storage systems.**

Started as a project to be used for managing page layout in a database storage engine. It outgrew into a data format for runtime-defined type system. It supports variery of primitve types and also nested arrays and tuples.

To reiterate : This project only provides primitive utility functions/structures to systematically organize your data in tuples of runtime-defined datatypes in a specific (slotted page or fixed array page) model for a block-storage-memory consisting of pages of predefined fixed size.

Note:: The TupleStore API deals with only 32 bit unsigned integers for most cases (sizes, offsets and indices). This imposes a hard limit of 4GB on the page size. But remember uint32_t can overflow, and adding overflow checks in TupleStore for every mathematical operation is a performance killer. So I want you, the users to back calculate, in a way similar to the following (shown below).
 * Lets say you want to support 3,000,000,000 byte page sizes (possibly 4GB) to be on safe side, to avoid overflows.
 * This could possibly allow you to build tuples with 54000 columns each with about 54000 bytes.
 * So impose a limit on 54000 columns with no column storing any more than 54000 inline bytes, inside your database.
 * and you are good to go, no overflows will be encountered whatsoever.

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
