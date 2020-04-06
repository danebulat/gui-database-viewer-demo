#ifndef UTILS_H
#define UTILS_H

#include <iostream>
using std::cout;
using std::endl;

// ------------------------------------------------------------
// Pointer Utilities
// ------------------------------------------------------------

template<class T> bool SafeDelete(T*& pointer) {
    if (pointer != NULL) {
        delete pointer;
        pointer = NULL;
        return true;
    }

    return false;
}

template <class T> bool SafeDeleteArray(T*& pointer) {
    
    /* Array allocated with syntax:
       Type* Pointer = new Type[numElements];
    */

    if (pointer != NULL) {
        delete [] pointer;
        pointer = NULL;
        return true;
    }

    return false;
}

#endif