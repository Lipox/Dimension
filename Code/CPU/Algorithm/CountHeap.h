#ifndef CPU_COUNTHEAP_H
#define CPU_COUNTHEAP_H

#include "Abstract.h"
#include "Heap.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CountHeap : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    CountHeap(uint32_t _MEMORY, std::string _name = "CountHeap"){
        this->name = _name;

        uint32_t LIGHT_MEMORY = _MEMORY * LIGHT_RATIO;
        uint32_t HEAVY_MEMORY = _MEMORY * HEAVY_RATIO;

        LENGTH = LIGHT_MEMORY / sizeof(COUNT_TYPE) / HASH_NUM;
        heap = new Heap<DATA_TYPE, COUNT_TYPE>(heap->Memory2Size(HEAVY_MEMORY));

        CSketch = new COUNT_TYPE* [HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM; ++i){
            CSketch[i] = new COUNT_TYPE[LENGTH];
            memset(CSketch[i], 0, sizeof(COUNT_TYPE) * LENGTH);
        }

    }

    ~CountHeap(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] CSketch[i];
        delete [] CSketch;
        delete heap;
    }

    void Insert(const DATA_TYPE item) {
        std::vector<COUNT_TYPE> result(HASH_NUM);

        for(uint32_t i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t polar = hash(item, i + HASH_NUM) & 1;

            CSketch[i][position] += delta[polar];
            result[i] = CSketch[i][position] * delta[polar];
        }

        std::sort(result.begin(), result.end());

        COUNT_TYPE est = (HASH_NUM & 1) ? result[HASH_NUM / 2] :
           ((result[HASH_NUM / 2] + result[HASH_NUM / 2 - 1]) / 2);

        heap->Insert(item, est);
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = heap->Query(item);
        if(ret != 0)
            return ret;

        std::vector<COUNT_TYPE> result(HASH_NUM);

        for(uint32_t i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            uint32_t polar = hash(item, i + HASH_NUM) & 1;

            result[i] = CSketch[i][position] * delta[polar];
        }

        std::sort(result.begin(), result.end());

        return (HASH_NUM & 1) ? result[HASH_NUM / 2] :
               ((result[HASH_NUM / 2] + result[HASH_NUM / 2 - 1]) / 2);
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        return heap->HHQuery(thres);
    }

private:

    const double HEAVY_RATIO = 0.25;
    const double LIGHT_RATIO = 0.75;

    const int32_t delta[2] = {+1, -1};

    uint32_t LENGTH;
    const uint32_t HASH_NUM = 2;

    COUNT_TYPE** CSketch;
    Heap<DATA_TYPE, COUNT_TYPE>* heap;
};


#endif //CPU_COUNTHEAP_H
