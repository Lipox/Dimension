#ifndef CPU_CUOURS_H
#define CPU_CUOURS_H

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CUOurs : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };

    CUOurs(uint32_t _MEMORY, uint32_t _HASH_NUM, std::string _name = "CUOurs"){
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

    ~CUOurs(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] counter[i];
        delete [] counter;
    }

    void Insert(const DATA_TYPE item){
        uint32_t* position = new uint32_t [HASH_NUM];
        COUNT_TYPE minimum = 0x7fffffff;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            position[i] = hash(item, i) % LENGTH;
            if(counter[i][position[i]].ID == item){
                counter[i][position[i]].count += 1;
                delete [] position;
                return;
            }
            else{
                minimum = MIN(minimum, counter[i][position[i]].count);
            }
        }

        for(uint32_t i = 0;i < HASH_NUM;++i){
            if(counter[i][position[i]].count == minimum){
                counter[i][position[i]].count += 1;
                if(randomGenerator() % counter[i][position[i]].count == 0){
                    counter[i][position[i]].ID = item;
                }
                delete [] position;
                return;
            }
        }

        std::cout << "Code Error" << std::endl;
        delete [] position;
        return;
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        for(uint32_t i = 0;i < HASH_NUM;++i){
            uint32_t position = hash(item, i) % LENGTH;
            if(counter[i][position].ID == item){
                return counter[i][position].count;
            }
        }
        return 0;
    }

    HashMap Merge(const DATA_TYPE mask){
        HashMap ret;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                ret[counter[i][j].ID & mask] += counter[i][j].count;
            }
        }

        return ret;
    };

private:
    uint32_t MEMORY;
    uint32_t LENGTH;
    uint32_t HASH_NUM;

    Counter** counter;
};

#endif //CPU_CUOUS_H
