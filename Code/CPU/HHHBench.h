#ifndef CPU_HHHBENCH_H
#define CPU_HHHBENCH_H

#include <vector>


#include "Univmon.h"
#include "Elastic.h"
#include "SpaceSaving.h"

#include "Ours.h"
#include "CUOurs.h"
#include "SimpleOurs.h"

#include "MMap.h"
#include "Timer.h"

#define HHH_TRAFFIC 5

template<typename DATA_TYPE,typename COUNT_TYPE>
class HHHBench{
public:

    typedef CUOurs<DATA_TYPE, COUNT_TYPE>* OurSketch;
    typedef SpaceSaving<DATA_TYPE, COUNT_TYPE>* Sketch;
    typedef std::vector<Sketch> SketchVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    HHHBench(const char* PATH, double alpha){
        result = Load(PATH);
        dataset = (DATA_TYPE*)result.start;
        length = result.length / sizeof(DATA_TYPE);

        for(uint64_t i = 0;i < length;++i){
            for(uint32_t j = 0;j < HHH_TRAFFIC;++j){
                mp[j][dataset[i] & MASK[j]] += 1;
            }
        }

        GetHHH(alpha * length);
    }

    ~HHHBench(){
        UnLoad(result);
    }

    void HHBench(uint32_t MEMORY, uint32_t K, double alpha){
        for(uint32_t i = 2;i <= K;++i){
            HHOursBench(MEMORY, i, alpha * length);
            HHOtherBench(MEMORY, i, alpha * length);
            RHHHBench(MEMORY, i, alpha * length);
        }
    }

    void HHOursBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        OurSketch sketch = new CUOurs<DATA_TYPE, COUNT_TYPE>(MEMORY);
        sketch->Insert(dataset, length);
        HHOurError(sketch, K, thres);
        delete sketch;
    }

    void HHOtherBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        SketchVector vec(K);

        for(uint32_t i = 0;i < K;++i){
            vec[i] = new SpaceSaving<DATA_TYPE, COUNT_TYPE>(MEMORY / K);
            OtherInsert(vec[i], dataset, length, i);
        }

        HHOtherError(vec, K, thres);

        for(uint32_t i = 0;i < K;++i){
            delete vec[i];
        }
    }

    void RHHHBench(uint32_t MEMORY, uint32_t K, uint32_t thres){
        SketchVector vec(K);

        for(uint32_t i = 0;i < K;++i){
            vec[i] = new SpaceSaving<DATA_TYPE, COUNT_TYPE>(MEMORY / K);
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

    HashMap mp[HHH_TRAFFIC];
    HashMap hhh[HHH_TRAFFIC];

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
            uint32_t d = randomGenerator() % K;
            vec[d]->Insert(data[i] & MASK[d]);
        }
    }

    void GetHHH(uint32_t thres){
        for(uint32_t i = 0;i < HHH_TRAFFIC;++i){
            HashMap temp;
            for(auto it = mp[i].begin();it != mp[i].end();++it){
                if(it->second > thres){
                    temp[it->first] = it->second;
                }
            }

            for(uint32_t j = 0;j < HHH_TRAFFIC;++j){
                for(auto it = hhh[j].begin();it != hhh[j].end();++it){
                    temp[it->first & MASK[i]] -= it->second;
                }
            }

            for(auto it = temp.begin();it != temp.end();++it){
                if(it->second > thres){
                    hhh[i][it->first] = it->second;
                }
            }
        }
    }

    void HHOurError(OurSketch sketch, uint32_t K, uint32_t thres){
        HashMap myHHH[HHH_TRAFFIC];

        for(uint32_t i = 0;i < K;++i){
            HashMap temp, ret = sketch->Merge(MASK[i]);

            for(auto it = ret.begin();it != ret.end();++it){
                if(it->second > thres){
                    temp[it->first] = it->second;
                }
            }

            for(uint32_t j = 0;j < i;++j){
                for(auto it = myHHH[j].begin();it != myHHH[j].end();++it){
                    temp[it->first & MASK[i]] -= it->second;
                }
            }

            for(auto it = temp.begin();it != temp.end();++it){
                if(it->second > thres){
                    myHHH[i][it->first] = it->second;
                }
            }
        }

        PrintError(sketch->name, myHHH, K);
    }

    void HHOtherError(SketchVector vec, uint32_t K, uint32_t thres){
        HashMap myLowerHHH[HHH_TRAFFIC], myHHH[HHH_TRAFFIC];

        for(uint32_t i = 0;i < K;++i){
            HashMap temp, ret = vec[i]->HHQuery(thres);
            HashMap lowerTemp = vec[i]->LHHQuery(thres);

            for(auto it = ret.begin();it != ret.end();++it){
                if(it->second > thres){
                    temp[it->first] = it->second;
                }
            }

            for(uint32_t j = 0;j < i;++j){
                for(auto it = myLowerHHH[j].begin();it != myLowerHHH[j].end();++it){
                    temp[it->first & MASK[i]] -= it->second;
                }
            }

            for(auto it = temp.begin();it != temp.end();++it){
                if(it->second > thres){
                    myHHH[i][it->first] = it->second;
                    COUNT_TYPE lower = it->second +
                                       lowerTemp[it->first] - ret[it->first];
                    if(lower < 0)
                        lower = 0;
                    myLowerHHH[i][it->first] = lower;
                }
            }
        }

        PrintError(vec[0]->name, myHHH, K);
    }

    void RHHHError(SketchVector vec, uint32_t K, uint32_t thres){
        HashMap myLowerHHH[HHH_TRAFFIC], myHHH[HHH_TRAFFIC];

        for(uint32_t i = 0;i < K;++i){
            HashMap temp, ret = vec[i]->HHQuery(thres / K);
            HashMap lowerTemp = vec[i]->LHHQuery(thres / K);

            for(auto it = ret.begin();it != ret.end();++it){
                if(it->second * K > thres){
                    temp[it->first] = it->second * K;
                }
            }

            for(uint32_t j = 0;j < i;++j){
                for(auto it = myLowerHHH[j].begin();it != myLowerHHH[j].end();++it){
                    temp[it->first & MASK[i]] -= it->second;
                }
            }

            for(auto it = temp.begin();it != temp.end();++it){
                if(it->second > thres){
                    myHHH[i][it->first] = it->second;
                    COUNT_TYPE lower = it->second +
                            (lowerTemp[it->first] - ret[it->first]) * K;
                    if(lower < 0)
                        lower = 0;
                    myLowerHHH[i][it->first] = lower;
                }
            }
        }

        PrintError(vec[0]->name, myHHH, K);
    }

    void PrintError(std::string name, HashMap* myHHH, uint32_t K){
        double aae = 0, are = 0, both = 0, hh = 0, record = 0;

        for(uint32_t i = 0;i < K;++i){
            hh += hhh[i].size();
            record += myHHH[i].size();

            for(auto it = hhh[i].begin();it != hhh[i].end();++it){
                if(myHHH[i].find(it->first) != myHHH[i].end()){
                    both += 1;
                    COUNT_TYPE estimated = myHHH[i][it->first];
                    aae += abs(it->second - estimated);
                    are += abs(it->second - estimated) / (double)it->second;
                }
            }
        }

        std::cout << name << std::endl;
        std::cout << "AAE: " << aae / both << std::endl;
        std::cout << "ARE: " << are / both << std::endl;
        std::cout << "CR: " << both / hh << std::endl;
        std::cout << "PR: " << both / record << std::endl;
    }
};

#endif //CPU_HHHBENCH_H
