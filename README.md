# TupleStore

This library provides you with a very simple 
 * **row oriented - slotted page & fixed array page storage models,**
 * **for fixed or variable length predefined tuples,**
 * **to be stored on fixed sized pages for block storage systems.**

Started as a project to be used for managing page layout in a database storage engine.

To reiterate : This project is not a database, embedded database, database storage engine or any of that.
It only provides primitive utility functions/structures to systematically organize your data in tuples of predefined datatypes in a specific (slotted page or fixed array page) model for a block storage memory consisting of pages of predefined fixed size.

## Setup instructions
**Install dependencies :**
 * [Cutlery](https://github.com/RohanVDvivedi/Cutlery)

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/TupleStore.git`

**Build from source :**
 * `cd TupleStore`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-ltuplestore` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<tuple_def.h>`
   * `#include<tuple.h>`
   * `#include<page_layout.h>`
   * `#include<int_accesses.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd TupleStore`
 * `sudo make uninstall`
