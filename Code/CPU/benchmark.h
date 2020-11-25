#ifndef WINDOW_SKETCH_BENCHMARK_H
#define WINDOW_SKETCH_BENCHMARK_H

#include <vector>

//#include "RHHH.h"
#include "Univmon.h"
#include "Elastic.h"
//#include "BeauCoup.h"
#include "SpaceSaving.h"

#include "Ours.h"
#include "CUOurs.h"
#include "SimpleOurs.h"

#include "MMap.h"
#include "Timer.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef Abstract<DATA_TYPE, COUNT_TYPE>* Sketch;
    typedef std::vector<Sketch> SketchVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    BenchMark(const char* PATH){
        result = Load(PATH);
        dataset = (DATA_TYPE*)result.start;
        length = result.length / sizeof(DATA_TYPE);

        for(uint64_t i = 0;i < length;++i){
            mp[dataset[i]] += 1;
        }
    }

    ~BenchMark(){
        UnLoad(result);
    }

    void Parameter(double alpha){
        SketchVector sketches = {
                new UnivMon<DATA_TYPE, COUNT_TYPE>(2000000),
                new CUOurs<DATA_TYPE, COUNT_TYPE>(2000000, 2),
                new Elastic<DATA_TYPE, COUNT_TYPE>(2000000),
                new SpaceSaving<DATA_TYPE, COUNT_TYPE>(2000000),
       };

        for(auto sketch : sketches){
            SketchThp(sketch);
            FECheckError(sketch);
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
        SketchInsert(sketch, dataset, length);
        finish = now();

        cout << sketch->name << endl;
        cout << "Thp: " << length / durationms(finish, start) << endl;
    }

    inline void SketchInsert(Sketch sketch, DATA_TYPE* data, uint32_t size){
        for(uint32_t i = 0;i < size;++i){
            sketch->Insert(data[i]);
        }
    }

    void FECheckError(Sketch sketch){
        double aae = 0, are = 0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE estimated = sketch->Query(it->first);
            aae += abs(it->second - estimated);
            are += abs(it->second - estimated) / (double)it->second;
        }

        cout << sketch->name << endl;
        cout << "AAE: " << aae / mp.size() << endl;
        cout << "ARE: " << are / mp.size() << endl;
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

        cout << sketch->name << endl;
        cout << "AAE: " << aae / both << endl;
        cout << "ARE: " << are / both << endl;
        cout << "CR: " << both / hh << endl;
        cout << "PR: " << both / ret.size() << endl;
    }


};

#endif //WINDOW_SKETCH_BENCHMARK_H
