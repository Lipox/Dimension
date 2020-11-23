#ifndef CPU_SIMPLEOURS_H
#define CPU_SIMPLEOURS_H

#include "Abstract.h"

//only one row

template<typename DATA_TYPE,typename COUNT_TYPE>
class SimpleOurs : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };

    SimpleOurs(uint32_t _MEMORY, std::string _name = "SimpleOurs"){
        this->name = _name;

        MEMORY = _MEMORY;
        LENGTH = MEMORY / sizeof(Counter);

        counter = new Counter[LENGTH];
        memset(counter, 0, sizeof(Counter) * LENGTH);
    }

    ~SimpleOurs(){
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        uint32_t position = hash(item) % LENGTH;
        counter[position].count += 1;
        if(randomGenerator() % counter[position].count == 0){
            counter[position].ID = item;
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        uint32_t position = hash(item) % LENGTH;
        return counter[position].ID == item? counter[position].count : 0;
    }

    COUNT_TYPE HHQuery(const DATA_TYPE item){
        return Query(item);
    }

    HashMap Merge(const DATA_TYPE mask){
        HashMap ret;

        for(uint32_t j = 0;j < LENGTH;++j){
            ret[counter[j].ID & mask] += counter[j].count;
        }

        return ret;
    };

private:
    uint32_t MEMORY;
    uint32_t LENGTH;

    Counter* counter;
};

#endif //CPU_SIMPLEOURS_H
