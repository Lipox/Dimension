#ifndef CPU_COUNTHEAP_H
#define CPU_COUNTHEAP_H

#include "CSketch.h"
#include "Heap.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CountHeap : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    CountHeap(uint32_t _MEMORY, std::string _name = "CountHeap"){
        this->name = _name;

        uint32_t LIGHT_MEMORY = _MEMORY * LIGHT_RATIO;
        uint32_t HEAVY_MEMORY = _MEMORY * HEAVY_RATIO;

        sketch = new CSketch<DATA_TYPE, COUNT_TYPE>(LIGHT_MEMORY);
        heap = new Heap<DATA_TYPE, COUNT_TYPE>(heap->Memory2Size(HEAVY_MEMORY));
    }

    ~CountHeap(){
        delete sketch;
        delete heap;
    }

    void Insert(const DATA_TYPE item) {
        sketch->Insert(item);
        COUNT_TYPE est = sketch->Query(item);
        heap->Insert(item, est);
    }

    void Insert(DATA_TYPE* dataset, uint64_t length){
        for(uint64_t i = 0;i < length;++i){
            Insert(dataset[i]);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = heap->Query(item);
        if(ret != 0)
            return ret;
        return sketch->Query(item);
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        return heap->HHQuery(thres);
    }

    HashMap AllQuery(){
        return heap->AllQuery();
    }

private:

    const double HEAVY_RATIO = 0.25;
    const double LIGHT_RATIO = 0.75;

    CSketch<DATA_TYPE, COUNT_TYPE>* sketch;
    Heap<DATA_TYPE, COUNT_TYPE>* heap;
};


#endif //CPU_COUNTHEAP_H
