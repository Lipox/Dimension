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

    virtual void Insert(const DATA_TYPE item) = 0;
    virtual COUNT_TYPE Query(const DATA_TYPE item) = 0;
    virtual COUNT_TYPE HHQuery(const DATA_TYPE item) = 0;
    virtual HashMap Merge(const DATA_TYPE mask){return HashMap();};
};

#endif //WINDOW_SKETCH_ABSTRACT_H
