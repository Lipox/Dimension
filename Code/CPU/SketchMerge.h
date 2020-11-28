#ifndef CPU_SKETCHMERGE_H
#define CPU_SKETCHMERGE_H

template<typename DATA_TYPE, typename COUNT_TYPE, typename Sketch>
class SketchMerge : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SketchMerge(uint32_t _K, uint32_t _MEMORY){
        K = _K;
        sketches = new Sketch* [K];
        for (uint32_t i = 0;i < K;++i) {
            sketches[i] = new Sketch(_MEMORY);
        }
        this->name = sketches[0]->name + "-SketchMerge";
    }

    ~SketchMerge(){
        for (uint32_t i = 0;i < K;++i) {
            delete sketches[i];
        }
        delete [] sketches;
    }

    void Insert(DATA_TYPE* dataset, uint64_t length) {
/*
        for (uint32_t i = 0;i < length;++i) {
            uint32_t pos = hash(dataset[i], 19) % K;
            sketches[pos]->Insert(dataset[i]);
        }
*/

        for (uint32_t i = 0;i < K;++i) {
            uint32_t start = i * length / K;
            uint32_t end = (i + 1) * length / K;
            if(i == K - 1)
                end = length;

            for(uint32_t pos = start;pos < end;++pos){
                sketches[i]->Insert(dataset[pos]);
            }
        }

    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = 0;
        for (uint32_t i = 0;i < K;++i) {
            ret += sketches[i]->Query(item);
        }
        return ret;
    }

    HashMap HHQuery(const COUNT_TYPE thres){
        HashMap ret, hhSet;

        for (uint32_t i = 0;i < K;++i) {
            HashMap temp = sketches[i]->AllQuery();
            for(auto it = temp.begin();it != temp.end();++it){
                if(hhSet.find(it->first) == hhSet.end()){
                    hhSet[it->first] = 1;
                    COUNT_TYPE estimate = Query(it->first);
                    if(estimate > thres)
                        ret[it->first] = estimate;
                }
            }
        }

        return ret;
    }

    HashMap AllQuery(){
        return HashMap();
    }

private:
    uint32_t K;

    Sketch** sketches;
};

#endif //CPU_SKETCHMERGE_H
