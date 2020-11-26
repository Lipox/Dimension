#ifndef CPU_UNIVMON_H
#define CPU_UNIVMON_H

#include "Abstract.h"
#include "CountHeap.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class UnivMon : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;
    
    UnivMon(uint32_t _MEMORY, std::string _name = "UnivMon"){
        this->name = _name;

        sketches = new CountHeap<DATA_TYPE, COUNT_TYPE>* [LEVEL];

        for(uint32_t i = 0;i < LEVEL; ++i){
            sketches[i] = new CountHeap<DATA_TYPE, COUNT_TYPE>(_MEMORY >> (i + 1));
        }
    }

    ~UnivMon(){
        for(uint32_t i = 0;i < LEVEL;++i){
            delete sketches[i];
        }
        delete [] sketches;
    }

    void Insert(DATA_TYPE item){
        uint32_t pos = hash(item, 199);
        sketches[0]->Insert(item);
        for(uint32_t i = 1; i < LEVEL; ++i){
            if(pos & 1)
                sketches[i]->Insert(item);
            else
                break;
            pos >>= 1;
        }
    }

    void Insert(DATA_TYPE* dataset, uint64_t length){
        for(uint64_t i = 0;i < length;++i){
            Insert(dataset[i]);
        }
    }

    COUNT_TYPE Query(DATA_TYPE item){
        uint32_t pos = hash(item, 199);
        int32_t level;

        for(level = 1; level < LEVEL; ++level){
            if(pos & 1)
                pos >>= 1;
            else
                break;
        }

        COUNT_TYPE ret = sketches[level - 1]->Query(item);
        for(int32_t i = level - 2;i >= 0;--i){
            ret = 2 * ret - sketches[i]->Query(item);
        }

        return ret;
    }

    HashMap HHQuery(COUNT_TYPE thres){
        HashMap ret, hhSet;

        for(uint32_t i = 0;i < LEVEL;++i){
            HashMap temp = sketches[i]->AllQuery();
            for(auto it = temp.begin();it != temp.end();++it){
                if(hhSet.find(it->first) == hhSet.end()){
                    hhSet[it->first] = it->second;
                    COUNT_TYPE estimate = Query(it->first);
                    if(estimate > thres)
                        ret[it->first] = estimate;
                }
            }
        }

        return ret;
    }

    HashMap AllQuery(){
        HashMap ret, hhSet;

        for(uint32_t i = 0;i < LEVEL;++i){
            HashMap temp = sketches[i]->AllQuery();
            for(auto it = temp.begin();it != temp.end();++it){
                if(hhSet.find(it->first) == hhSet.end()){
                    hhSet[it->first] = it->second;
                    ret[it->first] = Query(it->first);
                }
            }
        }

        return ret;
    }

private:
    const uint32_t LEVEL = 6;
    CountHeap<DATA_TYPE, COUNT_TYPE>** sketches;
};

#endif //CPU_UNIVMON_H