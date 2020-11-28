#ifndef DISTRIBUTE_OURS_H
#define DISTRIBUTE_OURS_H

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class Ours : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };

    Ours(uint32_t _MEMORY, uint32_t _HASH_NUM = 2, std::string _name = "Ours"){
        this->name = _name;

        HASH_NUM = _HASH_NUM;
        LENGTH = _MEMORY / HASH_NUM / sizeof(Counter);

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

    void Insert(DATA_TYPE* dataset, uint64_t length){
        for(uint64_t i = 0;i < length;++i){
            Insert(dataset[i]);
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

    HashMap HHQuery(const COUNT_TYPE thres){
        HashMap ret;
        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                if(ret.find(counter[i][j].ID) == ret.end()){
                    COUNT_TYPE temp = Query(counter[i][j].ID);
                    if(temp > thres){
                        ret[counter[i][j].ID] = temp;
                    }
                }
            }
        }
        return ret;
    }

    HashMap AllQuery(){
        HashMap ret;
        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                if(ret.find(counter[i][j].ID) == ret.end()){
                    ret[counter[i][j].ID] = Query(counter[i][j].ID);
                }
            }
        }
        return ret;
    }

    HashMap Merge(DATA_TYPE MASK){
        HashMap ret, used;

        for(uint32_t i = 0;i < HASH_NUM;++i){
            for(uint32_t j = 0;j < LENGTH;++j){
                if(used.find(counter[i][j].ID) == used.end()){
                    used[counter[i][j].ID] = 1;
                    DATA_TYPE ID = counter[i][j].ID & MASK;
                    ret[ID] += Query(counter[i][j].ID);
                }
            }
        }

        return ret;
    };

private:
    uint32_t LENGTH;
    uint32_t HASH_NUM;

    Counter** counter;
};

#endif //DISTRIBUTE_OURS_H
