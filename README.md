# SimpleTupleStorageModel

This library provides you with a very simple 
 * **row oriented - slotted page storage model,**
 * **fixed length predefined tuples,**
 * **to be stored on fixed sized pages for block storage systems.**

Started as a project to be used in a small database storage engine.

To reiterate : This project is not a database, embedded database, database storage engine or any of that.
It only provides primitive utility functions to systematically organize your data in tuples of predefined datatype in a specific model for a block storage memory of predefined size.

## Setup instructions
**Install dependencies :**
 * There are no dependencies for this project.

**Download source code :**
 * `git clone https://github.com/RohanVDvivedi/SimpleTupleStorageModel.git`

**Build from source :**
 * `cd SimpleTupleStorageModel`
 * `make clean all`

**Install from the build :**
 * `sudo make install`
 * ***Once you have installed from source, you may discard the build by*** `make clean`

## Using The library
 * add `-lstupstom` linker flag, while compiling your application
 * do not forget to include appropriate public api headers as and when needed. this includes
   * `#include<tuple.h>`
   * `#include<tuple.h>`

## Instructions for uninstalling library

**Uninstall :**
 * `cd SimpleTupleStorageModel`
 * `sudo make uninstall`
