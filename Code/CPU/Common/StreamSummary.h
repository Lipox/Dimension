#ifndef BENCHMARK_STREAMSUMMARY_H
#define BENCHMARK_STREAMSUMMARY_H

//StreamSummary: variant of double linked list

#include "CuckooMap.h"

template<typename ID_TYPE>
class Node{
public:
    ID_TYPE ID;
    Node* prev;
    Node* next;

    Node(ID_TYPE _ID): ID(_ID), prev(nullptr), next(nullptr){}

    void Delete(){
        Connect(prev, next);
    }

    void Connect(Node* prev, Node* next){
        if(prev)
            prev->next = next;
        if(next)
            next->prev = prev;
    }
};

template<typename DATA_TYPE, typename COUNT_TYPE>
class StreamSummary{
public:

    class DataNode;
    class CountNode;

    class DataNode : public Node<DATA_TYPE>{
    public:
        CountNode* pCount;
        DataNode(DATA_TYPE _ID):Node<DATA_TYPE>(_ID),pCount(nullptr){}
    };

    class CountNode : public Node<COUNT_TYPE>{
    public:
        DataNode* pData;
        CountNode(COUNT_TYPE _ID = 0):Node<COUNT_TYPE>(_ID),pData(nullptr){}
    };

    typedef CuckooMap<DATA_TYPE, DataNode*> HashMap;

    StreamSummary(uint32_t _MEMORY){
	    SIZE = _MEMORY / (3 * sizeof(DATA_TYPE) + 5 * sizeof(void*) + sizeof(COUNT_TYPE));
        mp = new HashMap(SIZE);
        min = new CountNode();
    }

    ~StreamSummary(){
        delete mp;
        CountNode* pCount = min;
        while(pCount){

            DataNode* pData = pCount->pData;
            while(pData){
                DataNode* nextData = (DataNode*)pData->next;
                delete pData;
                pData = nextData;
            }

            CountNode* nextCount = (CountNode*)pCount->next;
            delete pCount;
            pCount = nextCount;
        }
    }

    uint32_t SIZE;
    HashMap* mp;
    CountNode* min;

    inline bool isFull(){
        return mp->size() >= SIZE;
    }

    inline void New_Data(const DATA_TYPE& data){
        DataNode* pData = new DataNode(data);
        Add_Count(min, pData);
        mp->Insert(data, pData);
    }

    inline void Add_Min(){
        Add_Data(((CountNode*)min->next)->pData->ID);
    }

    void Decrease_Min(){
        CountNode* pCount = (CountNode*)min->next;
        DataNode* pData = pCount->pData;

        CountNode* add = new CountNode(pCount->ID - 1);
        pCount->Connect(min, add);
        pCount->Connect(add, pCount);

        pCount->pData = (DataNode*)pData->next;
        pData->Delete();
        if(!pData->next){
            pCount->Delete();
            delete pCount;
        }

        pData->prev = pData->next = nullptr;
        pData->pCount = add;
        add->pData = pData;
    }

    void Add_Data(const DATA_TYPE& data){
        DataNode* pData = (*mp)[data];
        CountNode* pCount = pData->pCount;

        bool del = false;
        pData->Delete();
        if(pCount->pData == pData){
            pCount->pData = (DataNode*)pData->next;
            del = !pData->next;
        }

        Add_Count(pCount, pData);

        if(del){
            pCount->Delete();
            delete pCount;
        }
    }

    void Add_Count(CountNode* pCount, DataNode* pData){
        if(!pCount->next)
            pCount->Connect(pCount, new CountNode(pCount->ID + 1));
        else if(pCount->next->ID - pCount->ID > 1){
            CountNode* add = new CountNode(pCount->ID + 1);
            pCount->Connect(add, pCount->next);
            pCount->Connect(pCount, add);
        }

        pData->prev = nullptr;
        pData->pCount = (CountNode*)pCount->next;
        pData->Connect(pData, pData->pCount->pData);
        pData->pCount->pData = pData;
    }

    void SS_Replace(const DATA_TYPE& data){
        CountNode* pCount = (CountNode*)min->next;
        DataNode* pData = new DataNode(data);

        mp->Insert(data, pData);
        Add_Count(pCount, pData);
        pData = pCount->pData;
        pCount->pData = (DataNode*)pData->next;
        pData->Delete();
        if(!pData->next){
            pCount->Delete();
            delete pCount;
        }
        mp->Delete(pData->ID);
        delete pData;
    }
};

#endif //BENCHMARK_STREAMSUMMARY_H
