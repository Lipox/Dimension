#ifndef CPU_BEAUCOUP_H
#define CPU_BEAUCOUP_H

#include <math.h>
#include <bitset>

#include "Abstract.h"

#define BITLEN 128

template<typename DATA_TYPE,typename COUNT_TYPE>
class BeauCoup : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;
    typedef std::unordered_map<DATA_TYPE, std::bitset<BITLEN>> Table;

    BeauCoup(uint32_t _MEMORY, std::string _name = "BeauCoup"){
        this->name = _name;
        timestamp = 0;
    }

    ~BeauCoup(){
        std::cout << mp.size() * 2 * (sizeof(DATA_TYPE) + BITLEN / 8) << std::endl;
    }

    void Insert(const DATA_TYPE item){
        timestamp += 1;

        if(hash(timestamp) % sampleRate == 0){
            mp[item][hash(timestamp, 17) % BITLEN] = 1;
        }

        return;
    }

    void Insert(DATA_TYPE* dataset, uint64_t length){
        for(uint64_t i = 0;i < length;++i){
            Insert(dataset[i]);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        if(mp.find(item) != mp.end()){
            return Count(mp[item]) * sampleRate;
        }
        return 0;
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        HashMap ret;
        for(auto it = mp.begin(); it != mp.end();++it){
            COUNT_TYPE estimated = Count(it->second);
            if(estimated * sampleRate > thres)
                ret[it->first] = estimated * sampleRate;
        }
        return ret;
    }

    HashMap AllQuery(){
        HashMap ret;
        for(auto it = mp.begin(); it != mp.end();++it){
            ret[it->first] = Count(it->second) * sampleRate;
        }
        return ret;
    }

private:
    Table mp;

    uint32_t sampleRate = 128;
    uint64_t timestamp;

    inline COUNT_TYPE Count(std::bitset<BITLEN> bits){
        double count = bits.count();
        if(count >= BITLEN)
            count = BITLEN - 1;
        return - BITLEN * log(1.0 - count / BITLEN);
    }
};

#endif //CPU_BEAUCOUP_H
