# SimpleTupleStorageModel

This library provides you with a very simple 
 * row store oriented,
 * fixed length predefined tuple storage model,
 * on fixed sized pages for block storage systems,
 * which are compressed using ztd compression library.

Started as a project to be used in a small database storage engine.

To reiterate : This project is not a database, embedded database, database storage engine or any of that.
It only provides primitive utility functions to systematically organize your data in a specific model for a block storage memory.

setup instructions
 * cd SimpleTupleStorageModel
 * make clean
 * make all
 * eval `make path`