#ifndef WINDOW_SKETCH_BENCHMARK_H
#define WINDOW_SKETCH_BENCHMARK_H

#include <vector>

#include "Algorithm/Count.h"
#include "Algorithm/Count-Min.h"
#include "Algorithm/Ours.h"

#include "Common/MMap.h"
#include "Common/Timer.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef std::vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    BenchMark(const char* PATH){
        result = Load(PATH);
        start = (DATA_TYPE*)result.start;
        length = result.length / sizeof(DATA_TYPE);

        for(uint64_t i = 0;i < length;++i){
            mp[start[i]] += 1;
        }
    }

    ~BenchMark(){
        UnLoad(result);
    }

    void SketchError(uint32_t K){
        uint32_t interval = length / K;

        vector<AbsVector> algs(K);
        for(uint32_t i = 0;i < K;++i){
            algs[i] = {
                //new CM<DATA_TYPE, COUNT_TYPE>(1000000, 1),
                //new Count<DATA_TYPE, COUNT_TYPE>(1000000, 1),
                new Ours<DATA_TYPE, COUNT_TYPE>(500000, 1),
                new Priority<DATA_TYPE, COUNT_TYPE>(500000, 1),
                //new Ours<DATA_TYPE, COUNT_TYPE>(2000000, 1),
            };

            if(i == K - 1){
                BenchInsert(algs[i], &start[interval * i], length - interval * (K - 1));
            }
            else{
                BenchInsert(algs[i], &start[interval * i], interval);
            }
        }

        CheckError(algs, K);
        std::cout << K << ": End" << endl << endl;
    }

private:
    LoadResult result;

    DATA_TYPE* start;
    uint64_t length;

    HashMap mp;

    void BenchInsert(AbsVector sketches, DATA_TYPE* data, uint32_t size){
        for(uint32_t i = 0;i < size;++i){
            for(auto sketch : sketches)
                sketch->Insert(data[i]);
        }
    }

    void CheckError(vector<AbsVector> algs, uint32_t K){
        uint32_t size = algs[0].size();

        for(uint32_t i = 0;i < size;++i){
            double aae = 0, are = 0, number = 0;

            for(auto it = mp.begin();it != mp.end();++it){
                if(it->second > 100){
                    number += 1;

                    COUNT_TYPE estimated = 0;
                    for(uint32_t j = 0;j < K;++j) {
                        estimated += algs[j][i]->Query(it->first);
                    }

                    aae += abs(it->second - estimated);
                    are += abs(it->second - estimated) / (double)it->second;
                }
            }

            std::cout << "AAE: " << aae / number << std::endl
                      << "ARE: " << are / number << std::endl;
        }
    }
};

#endif //WINDOW_SKETCH_BENCHMARK_H
