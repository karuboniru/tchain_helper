## a header file to simplify reading ROOT file(s)

```
#include <chain_helper.h>
...
{
    ...
    std::vector<std::string> file_list{"/path/to/file1.root", "/path/to/file2.root"};
    root_chain<int, int[128]> chain(file_list, "tree_name", {"branch_name1", "branch_name2"});
    //         ^                                           ^
    //         |                                           |
    //         |                                           |
    //         |                                           \-- names for each branch in the tree
    //         |                                            
    //         \-- type of each branch
    for (const auto &[var1, var2]: chain) { // loop on all entries in the tree
        ... // free to use those variables
    }                           

    const auto & [var1, var2] = chain[NUM];
    // also, get variable in chain with certain id
    ...
    // no write support (yet) 
    ...
}