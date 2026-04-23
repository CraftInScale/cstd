# Library

C++ compilable with C-style code.<br>
Supports Linux.

# Use case

```
#include "std/cstd.h"

int main()
{
    Logger global_logger;
    create_logger(global_logger, "Main Thread", LOG_DEFAULT_LOG_FORMAT, LL_DEBUG);
    cstd_init(global_logger);
    
    // your code
    
    cstd_deinit();
    
    return 0;
}
```

# Dependencies

pthreads<br>
CMake >=3.12

# Building

1. Install dependencies

```
sudo apt-get install cmake ninja g++
```
