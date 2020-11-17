#ifndef SS_SS_H
#define SS_SS_H

#include "Heap.h"
#include "StreamSummary.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class SpaceSaving : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    SpaceSaving(uint32_t _MEMORY, std::string _name = "SpaceSaving"){
        this->name = _name;

        summary = new StreamSummary<DATA_TYPE, COUNT_TYPE>(_MEMORY);
    }

    ~SpaceSaving(){
        delete summary;
    }

    void Insert(const DATA_TYPE item){
        if(summary->mp->Lookup(item))
            summary->Add_Data(item);
        else{
            if(summary->isFull())
                summary->SS_Replace(item);
            else
                summary->New_Data(item);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        auto mp = summary->mp;
        return mp->Lookup(item)?  (*mp)[item]->pCount->ID : 0;
    }

private:
    StreamSummary<DATA_TYPE, COUNT_TYPE>* summary;
};

#endif //SS_SS_H
