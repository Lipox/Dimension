#ifndef DISTRIBUTE_OURS_H
#define DISTRIBUTE_OURS_H

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Ours : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };

    Ours(uint32_t _MEMORY, uint32_t _HASH_NUM, std::string _name = "Ours"){
        this->name = _name;

        MEMORY = _MEMORY;
        HASH_NUM = _HASH_NUM;
        LENGTH = MEMORY / HASH_NUM / sizeof(Counter);

        counter = new Counter*[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new Counter[LENGTH];
            memset(counter[i], 0, sizeof(Counter) * LENGTH);
        }
    }

    ~Ours(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] counter[i];
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            counter[i][position].count += 1;
            if(randomGenerator() % counter[i][position].count == 0){
                counter[i][position].ID = item;
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = 0;
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            if(counter[i][position].ID == item){
                ret += counter[i][position].count;
            }
        }
        return ret / HASH_NUM;
    }

private:
    uint32_t MEMORY;
    uint32_t LENGTH;
    uint32_t HASH_NUM;

    Counter** counter;
};

#endif //DISTRIBUTE_OURS_H
