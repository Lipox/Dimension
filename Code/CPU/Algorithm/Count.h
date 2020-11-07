#ifndef DISTRIBUTE_COUNT_H
#define DISTRIBUTE_COUNT_H

#include <vector>
#include <algorithm>

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Count : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    Count(uint32_t _LENGTH, uint32_t _HASH_NUM):
            LENGTH(_LENGTH), HASH_NUM(_HASH_NUM){
        counter = new COUNT_TYPE*[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new COUNT_TYPE[LENGTH];
            memset(counter[i], 0, sizeof(COUNT_TYPE) * LENGTH);
        }
    }

    ~Count(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] counter[i];
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t choice = hash(item, i + 101) & 1;
            counter[i][position] += delta[choice];
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        std::vector<COUNT_TYPE> result(HASH_NUM);
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t choice = hash(item, i + 101) & 1;
            result[i] = counter[i][position] * delta[choice];
        }
        return Median(result);
    }

private:
    const uint32_t LENGTH;
    const uint32_t HASH_NUM;

    COUNT_TYPE** counter;
    const COUNT_TYPE delta[2] = {+1, -1};

    COUNT_TYPE Median(std::vector<COUNT_TYPE> vec){
        std::sort(vec.begin(), vec.end());

        uint32_t size = vec.size();
        return (size & 1) ? vec[size >> 1] :
               (vec[size >> 1] + vec[(size >> 1) - 1]) / 2;
    }
};

#endif //DISTRIBUTE_COUNT_H
