#ifndef WINDOW_SKETCH_ABSTRACT_H
#define WINDOW_SKETCH_ABSTRACT_H

#include <string.h>
#include <unordered_map>
#include <iostream>

#include "hash.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Abstract{
public:
    Abstract(){}
    virtual ~Abstract(){};

    std::string name;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    virtual void Insert(DATA_TYPE item){};
    virtual void Insert(DATA_TYPE* dataset, uint64_t length) = 0;
    virtual COUNT_TYPE Query(const DATA_TYPE item) = 0;
    virtual HashMap HHQuery(const COUNT_TYPE thres) = 0;
    virtual HashMap AllQuery() = 0;
};

#endif //WINDOW_SKETCH_ABSTRACT_H
