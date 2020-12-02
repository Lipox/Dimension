#ifndef WINDOW_SKETCH_BENCHMARK_H
#define WINDOW_SKETCH_BENCHMARK_H

#include <vector>


#include "Univmon.h"
#include "Elastic.h"
#include "SpaceSaving.h"

#include "Ours.h"
#include "CUOurs.h"
#include "SimpleOurs.h"

#include "SketchMerge.h"

#include "MMap.h"
#include "Timer.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class SketchBench{
public:

    typedef Abstract<DATA_TYPE, COUNT_TYPE>* Sketch;
    typedef std::vector<Sketch> SketchVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SketchBench(const char* PATH){
        result = Load(PATH);
        dataset = (DATA_TYPE*)result.start;
        length = result.length / sizeof(DATA_TYPE);

        for(uint64_t i = 0;i < length;++i){
            mp[dataset[i]] += 1;
        }
    }

    ~SketchBench(){
        UnLoad(result);
    }

    void FEBench(uint32_t MEMORY){
        SketchVector sketches = {
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),

                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CSketch<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),

                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),
       };

        for(auto sketch : sketches){
            SketchThp(sketch);
            FECheckError(sketch);
            delete sketch;
        }
    }

    void HHBench(uint32_t MEMORY, double alpha){
        SketchVector sketches = {
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, Elastic<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),

                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, UnivMon<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),

                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(2, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(3, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(4, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(5, MEMORY),
                new SketchMerge<DATA_TYPE, COUNT_TYPE, CUOurs<DATA_TYPE, COUNT_TYPE>>(6, MEMORY),

                //new SketchMerge<DATA_TYPE, COUNT_TYPE, BeauCoup<DATA_TYPE, COUNT_TYPE>>(1, MEMORY),
        };

        for(auto sketch : sketches){
            SketchThp(sketch);
            HHCheckError(sketch, alpha * length);
            delete sketch;
        }
    }

private:
    LoadResult result;

    DATA_TYPE* dataset;
    uint64_t length;

    HashMap mp;


    void SketchThp(Sketch sketch){
        TP start, finish;

        start = now();
        sketch->Insert(dataset, length);
        finish = now();

        std::cout << sketch->name << std::endl;
        std::cout << "Thp: " << length / durationms(finish, start) << std::endl;
    }

    void FECheckError(Sketch sketch){
        double aae = 0, are = 0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE estimated = sketch->Query(it->first);
            aae += abs(it->second - estimated);
            are += abs(it->second - estimated) / (double)it->second;
        }

        std::cout << sketch->name << std::endl;
        std::cout << "AAE: " << aae / mp.size() << std::endl;
        std::cout << "ARE: " << are / mp.size() << std::endl;
    }

    void HHCheckError(Sketch sketch, uint32_t thres){
        double aae = 0, are = 0, both = 0, hh = 0;

        HashMap ret = sketch->HHQuery(thres);

        for(auto it = mp.begin();it != mp.end();++it){
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

        std::cout << sketch->name << std::endl;
        std::cout << "AAE: " << aae / both << std::endl;
        std::cout << "ARE: " << are / both << std::endl;
        std::cout << "CR: " << both / hh << std::endl;
        std::cout << "PR: " << both / ret.size() << std::endl;
    }

};

#endif //WINDOW_SKETCH_BENCHMARK_H
