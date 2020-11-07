#ifndef DISTRIBUTE_COUNT_MIN_H
#define DISTRIBUTE_COUNT_MIN_H

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CM : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    CM(uint32_t _LENGTH, uint32_t _HASH_NUM):
            LENGTH(_LENGTH), HASH_NUM(_HASH_NUM){
        counter = new COUNT_TYPE*[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new COUNT_TYPE[LENGTH];
            memset(counter[i], 0, sizeof(COUNT_TYPE) * LENGTH);
        }
    }

    ~CM(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] counter[i];
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            counter[i][position] += 1;
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = INT_MAX;
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            ret = MIN(ret, counter[i][position]);
        }
        return ret;
    }

private:
    const uint32_t LENGTH;
    const uint32_t HASH_NUM;

    COUNT_TYPE** counter;
};

#endif //DISTRIBUTE_COUNT_MIN_H
