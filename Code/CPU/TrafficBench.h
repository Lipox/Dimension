#ifndef CPU_TRAFFICBENCH_H
#define CPU_TRAFFICBENCH_H

#include <vector>


#include "Univmon.h"
#include "Elastic.h"
#include "SpaceSaving.h"
//#include "BeauCoup.h"

#include "Ours.h"
#include "CUOurs.h"
#include "SimpleOurs.h"

#include "MMap.h"
#include "Timer.h"

#define MAX_TRAFFIC 4

template<typename DATA_TYPE,typename COUNT_TYPE>
class TrafficBench{
public:

    typedef CUOurs<DATA_TYPE, COUNT_TYPE>* OurSketch;
    typedef Abstract<DATA_TYPE, COUNT_TYPE>* Sketch;
    typedef std::vector<Sketch> SketchVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    TrafficBench(const char* PATH){
        result = Load(PATH);
        dataset = (DATA_TYPE*)result.start;
        length = result.length / sizeof(DATA_TYPE);

        for(uint64_t i = 0;i < length;++i){
            for(uint32_t j = 0;j < MAX_TRAFFIC;++j){
                mp[j][dataset[i] & MASK[j]] += 1;
            }
        }
    }

    ~TrafficBench(){
        UnLoad(result);
    }

    void FEBench(uint32_t MEMORY, uint32_t K){
        for(uint32_t i = 1;i <= K;++i){
            FEOursBench(MEMORY, i);
            FEOtherBench<Elastic<DATA_TYPE, COUNT_TYPE>>(MEMORY, i);
            FEOtherBench<CSketch<DATA_TYPE, COUNT_TYPE>>(MEMORY, i);
        }
    }

    void HHBench(uint32_t MEMORY, uint32_t K, double alpha){
        for(uint32_t i = 1;i <= K;++i){
            HHOursBench(MEMORY, i, alpha * length);
            RHHHBench<SpaceSaving<DATA_TYPE, COUNT_TYPE>>(MEMORY, i, alpha * length);
            HHOtherBench<Elastic<DATA_TYPE, COUNT_TYPE>>(MEMORY, i, alpha * length);
            HHOtherBench<UnivMon<DATA_TYPE, COUNT_TYPE>>(MEMORY, i, alpha * length);
        }
    }

    void FEOursBench(uint32_t MEMORY, uint32_t K){
        OurSketch sketch = new CUOurs<DATA_TYPE, COUNT_TYPE>(MEMORY);
        sketch->Insert(dataset, length);
        FEOurError(sketch, K);
        delete sketch;
    }

    void HHOursBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        OurSketch sketch = new CUOurs<DATA_TYPE, COUNT_TYPE>(MEMORY);
        sketch->Insert(dataset, length);
        HHOurError(sketch, K, thres);
        delete sketch;
    }

    template<typename T>
    void FEOtherBench(uint32_t MEMORY, uint32_t K){
        SketchVector vec(K);

        for(uint32_t i = 0;i < K;++i){
            vec[i] = new T(MEMORY / K);
            OtherInsert(vec[i], dataset, length, i);
        }

        FEOtherError(vec, K);

        for(uint32_t i = 0;i < K;++i){
            delete vec[i];
        }
    }

    template<typename T>
    void HHOtherBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        SketchVector vec(K);

        for(uint32_t i = 0;i < K;++i){
            vec[i] = new T(MEMORY / K);
            OtherInsert(vec[i], dataset, length, i);
        }

        HHOtherError(vec, K, thres);

        for(uint32_t i = 0;i < K;++i){
            delete vec[i];
        }
    }

    template<typename T>
    void RHHHBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        SketchVector vec(K);

        for(uint32_t i = 0;i < K;++i){
            vec[i] = new T(MEMORY / K);
        }

        RHHHInsert(vec, K, dataset, length);
        RHHHError(vec, K, thres);

        for(uint32_t i = 0;i < K;++i){
            delete vec[i];
        }
    }

private:
    LoadResult result;

    DATA_TYPE* dataset;
    uint64_t length;

    HashMap mp[MAX_TRAFFIC];

    const uint64_t MASK[8] = {
            0xffffffffffffffffL, 0x00ffffffffffffffL,
            0x0000ffffffffffffL, 0x000000ffffffffffL,
            0x00000000ffffffffL, 0x0000000000ffffffL,
            0x000000000000ffffL, 0x00000000000000ffL};


    inline void OtherInsert(Sketch sketch, DATA_TYPE* data, uint64_t length, uint32_t pos) {
        for(uint32_t i = 0;i < length;++i){
            sketch->Insert(data[i] & MASK[pos]);
        }
    }

    inline void RHHHInsert(SketchVector vec, uint32_t K, DATA_TYPE* data, uint64_t length) {
        for(uint32_t i = 0;i < length;++i){
            uint32_t d = hash(data[i]) % K;
            vec[d]->Insert(data[i] & MASK[d]);
        }
    }

    void FEOurError(OurSketch sketch, uint32_t K){
        double aae = 0, are = 0, total = 0;

        for(uint32_t i = 0;i < K;++i){
            HashMap ret = sketch->Merge(MASK[i]);

            for(auto it = mp[i].begin();it != mp[i].end();++it){
                total += 1;
                COUNT_TYPE estimated = ret[it->first];
                aae += abs(it->second - estimated);
                are += abs(it->second - estimated) / (double)it->second;
            }
        }

        std::cout << sketch->name << std::endl;
        std::cout << "AAE: " << aae / total << std::endl;
        std::cout << "ARE: " << are / total << std::endl;
    }

    void HHOurError(OurSketch sketch, uint32_t K, uint32_t thres){
        double aae = 0, are = 0, both = 0, hh = 0, record = 0;

        for(uint32_t i = 0;i < K;++i){
            HashMap ret = sketch->Merge(MASK[i]);

            for(auto it = mp[i].begin();it != mp[i].end();++it){
                if(it->second > thres){
                    hh += 1;
                    if(ret.find(it->first) != ret.end()){
                        COUNT_TYPE estimated = ret[it->first];
                        if(estimated > thres){
                            both += 1;

                            aae += abs(it->second - estimated);
                            are += abs(it->second - estimated) / (double)it->second;
                        }
                    }
                }
            }

            for(auto it = ret.begin();it != ret.end();++it){
                if(it->second > thres){
                    record += 1;
                }
            }
        }

        std::cout << sketch->name << std::endl;
        std::cout << "AAE: " << aae / both << std::endl;
        std::cout << "ARE: " << are / both << std::endl;
        std::cout << "CR: " << both / hh << std::endl;
        std::cout << "PR: " << both / record << std::endl;
    }

    void FEOtherError(SketchVector vec, uint32_t K){
        double aae = 0, are = 0, total = 0;

        for(uint32_t i = 0;i < K;++i){
            for(auto it = mp[i].begin();it != mp[i].end();++it){
                total += 1;
                COUNT_TYPE estimated = vec[i]->Query(it->first);
                aae += abs(it->second - estimated);
                are += abs(it->second - estimated) / (double)it->second;
            }
        }

        std::cout << vec[0]->name << std::endl;
        std::cout << "AAE: " << aae / total << std::endl;
        std::cout << "ARE: " << are / total << std::endl;
    }

    void HHOtherError(SketchVector vec, uint32_t K, uint32_t thres){
        double aae = 0, are = 0, both = 0, hh = 0, record = 0;

        for(uint32_t i = 0;i < K;++i){
            HashMap ret = vec[i]->HHQuery(thres);

            for(auto it = mp[i].begin();it != mp[i].end();++it){
                if(it->second > thres){
                    hh += 1;
                    if(ret.find(it->first) != ret.end()){
                        both += 1;
                        COUNT_TYPE estimated = ret[it->first];
                        aae += abs(it->second - estimated);
                        are += abs(it->second - estimated) / (double)it->second;
                    }
                }
            }

            record += ret.size();
        }

        std::cout << vec[0]->name << std::endl;
        std::cout << "AAE: " << aae / both << std::endl;
        std::cout << "ARE: " << are / both << std::endl;
        std::cout << "CR: " << both / hh << std::endl;
        std::cout << "PR: " << both / record << std::endl;
    }

    void RHHHError(SketchVector vec, uint32_t K, uint32_t thres){
        double aae = 0, are = 0, both = 0, hh = 0, record = 0;

        for(uint32_t i = 0;i < K;++i){
            HashMap ret = vec[i]->HHQuery(thres / K);

            for(auto it = mp[i].begin();it != mp[i].end();++it){
                if(it->second > thres){
                    hh += 1;
                    if(ret.find(it->first) != ret.end()){
                        both += 1;
                        COUNT_TYPE estimated = ret[it->first] * K;
                        aae += abs(it->second - estimated);
                        are += abs(it->second - estimated) / (double)it->second;
                    }
                }
            }

            record += ret.size();
        }

        std::cout << vec[0]->name << std::endl;
        std::cout << "AAE: " << aae / both << std::endl;
        std::cout << "ARE: " << are / both << std::endl;
        std::cout << "CR: " << both / hh << std::endl;
        std::cout << "PR: " << both / record << std::endl;
    }

};

#endif //CPU_TRAFFICBENCH_H
