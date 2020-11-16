#ifndef CPU_RHHH_H
#define CPU_RHHH_H

#include "Abstract.h"

// RHHH schema (to be tested and modified)
#define NUM_MASKS 5
uint64_t masks[NUM_MASKS] = {
    0xFFFFFFFFu,
    0xFFFFFF00u,
    0xFFFF0000u,
    0xFF000000u,
    0x00000000u
};

template<typename DATA_TYPE,typename COUNT_TYPE>
class RHHH : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };

    RHHH(uint32_t _MEMORY, std::string _name = "RHHH"){
        this->name = _name;

        MEMORY = _MEMORY;
        HASH_NUM = 5;
        LENGTH = MEMORY / HASH_NUM / sizeof(Counter);

        counter = new Counter*[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            counter[i] = new Counter[LENGTH];
            memset(counter[i], 0, sizeof(Counter) * LENGTH);
        }
    }

    ~RHHH(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] counter[i];
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        uint32_t i = randomGenerator() % HASH_NUM;
        uint32_t position = hash(item & masks[i], i) % LENGTH;
        counter[i][position].ID = item & masks[i];
        counter[i][position].count += 1;
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = 0;
        uint32_t position = hash(item, 0) % LENGTH;
        if(counter[0][position].ID == item){
            ret += counter[0][position].count;
        }
        return ret;
    }

    COUNT_TYPE* Hierarchical_Query(const DATA_TYPE item){
        COUNT_TYPE *ret = new COUNT_TYPE[HASH_NUM];
        Counter *HHH=new Counter[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM;++i){
            ret[i] = 0;
        }
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item & masks[i], i) % LENGTH;
            if(counter[i][position].ID == item&masks[i]){
                ret[i] += counter[i][position].count;
                ret[i] += 2*Z*sqrt(N*V);                
                if(i > 0){
                    for(uint_32 k = 0; k < LENGTH; k++){
                        if(counter[i][k].ID&masks[i] == item&masks[i]){
                            for(int l=0;l<i;l++){
                                if(HHH[l].count==ret[l]){
                                    ret[i] -= ret[l];
                                }
                            }
                            break;
                        }
                    }
                }
                if(ret[i]>=theta*N){
                    HHH[i].ID = item;
                    HHH[i].count = ret[i];
                }
            }
        }
        return ret;
    }

private:
    uint32_t MEMORY;
    uint32_t LENGTH;
    uint32_t HASH_NUM;

    Counter** counter;
};

#endif //CPU_RHHH_H
