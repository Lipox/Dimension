#ifndef CPU_BEAUCOUP_H
#define CPU_BEAUCOUP_H

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class BeauCoup : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    BeauCoup(uint32_t _MEMORY, std::string _name = "BeauCoup"){
        this->name = _name;
        timestamp = 0;
    }

    ~BeauCoup(){
        uint32_t out = 0;
        for(auto it = mp.begin(); it != mp.end();++it){
            if(it->second > 128)
                out += 1;
        }
        std::cout << "Note: " << out << std::endl;
        std::cout << mp.size() * 2 * (sizeof(DATA_TYPE) + sizeof(COUNT_TYPE)) << std::endl;
    }

    void Insert(const DATA_TYPE item){
        timestamp += 1;

        if(hash(timestamp) % sampleRate == 0){
            mp[item] += 1;
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
            return mp[item] * sampleRate;
        }
        return 0;
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        HashMap ret;
        for(auto it = mp.begin(); it != mp.end();++it){
            if(it->second * sampleRate > thres)
                ret[it->first] = it->second * sampleRate;
        }
        return ret;
    }

    HashMap AllQuery(){
        return mp;
    }

private:
    HashMap mp;

    uint32_t sampleRate = 512;
    uint64_t timestamp;
};

#endif //CPU_BEAUCOUP_H
