#ifndef CPU_CSKETCH_H
#define CPU_CSKETCH_H

#include "Abstract.h"
#include "Heap.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CSketch : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    CSketch(uint32_t _MEMORY, std::string _name = "CountSketch"){
        this->name = _name;
        LENGTH = _MEMORY / sizeof(COUNT_TYPE) / HASH_NUM;

        sketch = new COUNT_TYPE* [HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM; ++i){
            sketch[i] = new COUNT_TYPE[LENGTH];
            memset(sketch[i], 0, sizeof(COUNT_TYPE) * LENGTH);
        }
    }

    ~CSketch(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] sketch[i];
        delete [] sketch;
    }

    void Insert(const DATA_TYPE item) {
        for(uint32_t i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t polar = hash(item, i + HASH_NUM) & 1;

            sketch[i][position] += delta[polar];
        }
    }

    void Insert(DATA_TYPE* dataset, uint64_t length){
        for(uint64_t i = 0;i < length;++i){
            Insert(dataset[i]);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        std::vector<COUNT_TYPE> result(HASH_NUM);

        for(uint32_t i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t polar = hash(item, i + HASH_NUM) & 1;

            result[i] = sketch[i][position] * delta[polar];
        }

        std::sort(result.begin(), result.end());

        COUNT_TYPE est = (HASH_NUM & 1) ? result[HASH_NUM / 2] :
                         ((result[HASH_NUM / 2] + result[HASH_NUM / 2 - 1]) / 2);
        return est;
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        return HashMap();
    }

    HashMap AllQuery(){
        return HashMap();
    }

private:
    const int32_t delta[2] = {+1, -1};

    uint32_t LENGTH;
    const uint32_t HASH_NUM = 2;

    COUNT_TYPE** sketch;
};

#endif //CPU_CSKETCH_H
