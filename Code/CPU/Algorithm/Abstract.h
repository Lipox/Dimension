#ifndef WINDOW_SKETCH_ABSTRACT_H
#define WINDOW_SKETCH_ABSTRACT_H

#include <string.h>
#include <unordered_map>

#include "hash.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Abstract{
public:
    Abstract(){}
    virtual ~Abstract(){};

    virtual void Insert(const DATA_TYPE item) = 0;
    virtual COUNT_TYPE Query(const DATA_TYPE item) = 0;
};

#endif //WINDOW_SKETCH_ABSTRACT_H
