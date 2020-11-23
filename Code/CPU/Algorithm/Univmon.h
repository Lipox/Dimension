#ifndef CPU_UNIVMON_H
#define CPU_UNIVMON_H

#include "Abstract.h"
using std::min;
using std::swap;

template<typename SORT_TYPE>
void Sort(SORT_TYPE* p1, SORT_TYPE* p2){
    if (p1 >= p2)  return;
    SORT_TYPE* i,* j;
    SORT_TYPE p, tmp; 
    i = p1;
    j = p2;                                                            
    p = *p1;                              

    while(i < j) {                                                   
        while (*i <= p)                                                                  
            i++;
        while (p <= *j)                                                                  
            j--;
        if ( i >= j)
            break;
        *i += *j;
        *j = *i - *j;
        *i -= *j;
    } 
    Sort(p1, i-1); 
    Sort(j+1, p2); 
}

template<typename DATA_TYPE,typename COUNT_TYPE>
class CountHeap : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };
    uint32_t MEMORY;
    uint32_t LENGTH;
    uint32_t HASH_NUM;
    uint32_t capacity;
    uint32_t heap_element_num;

    Counter * heap; 
    COUNT_TYPE ** c_sketch;
    std::unordered_map<DATA_TYPE, COUNT_TYPE> ht; 

    CountHeap(uint32_t _MEMORY, uint32_t _HASH_NUM, std::string _name = "CountHeap"){
        this->name = _name;
        MEMORY = _MEMORY;
        HASH_NUM = _HASH_NUM;
        heap_element_num = 0;
        LENGTH = MEMORY / HASH_NUM / sizeof(Counter) / 2;
        capacity = MEMORY / sizeof(Counter) / 4;

        c_sketch = new COUNT_TYPE*[HASH_NUM];
        for(uint32_t i = 0;i < HASH_NUM; ++i){
            c_sketch[i] = new COUNT_TYPE[LENGTH];
            memset(c_sketch[i], 0, sizeof(COUNT_TYPE) * LENGTH);
        }
        heap = new Counter[capacity];
        ht.clear();
    }

    ~CountHeap(){
        for(uint32_t i = 0;i < HASH_NUM;++i)
            delete [] c_sketch[i];
        delete [] c_sketch;
        delete [] heap;
        ht.clear();
    }

    double get_f2()
    {
        double res[HASH_NUM];
        for (int i = 0; i < HASH_NUM; ++i) {
            double est = 0;
            for (int j = 0; j < LENGTH; ++j) {
                est += double(c_sketch[i][j]) * double(c_sketch[i][j]);
            }
            res[i] = est;
        }

        Sort(res, res + LENGTH);
        if (HASH_NUM % 2) {
            return res[LENGTH / 2];
        } else {
            return (res[LENGTH / 2] + res[LENGTH / 2 - 1]) / 2;
        }
    }

    void heap_adjust_down(int i) {
        while (i <= heap_element_num / 2) {
            int l_child = 2 * i + 1;
            int r_child = 2 * i + 2;
            int larger_one = i;
            if (l_child < heap_element_num && heap[l_child].count < heap[larger_one].count) {
                larger_one = l_child;
            }
            if (r_child < heap_element_num && heap[r_child].count < heap[larger_one].count) {
                larger_one = r_child;
            }
            if (larger_one != i) {
                swap(heap[i], heap[larger_one]);
                swap(ht[heap[i].ID], ht[heap[larger_one].ID]);
                i = larger_one;
            } else {
                break;
            }
        }
    }

    void heap_adjust_up(int i) {  
        while (i >= 1) {
            int parent = (i - 1) / 2;
            if (heap[parent].count <= heap[i].count) {
                break;
            }
            swap(heap[i], heap[parent]);
            swap(ht[heap[i].ID], ht[heap[parent].ID]);
            i = parent;
        }
    }

    void Insert(const DATA_TYPE item) {
        COUNT_TYPE ans[LENGTH];
        for (int i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            int polar = hash(item, i + HASH_NUM) % 2;
            c_sketch[i][position] += polar ? 1 : -1;
            COUNT_TYPE val = c_sketch[i][position];
            ans[i] = polar ? val : -val;
        }
        Sort(ans, ans + LENGTH);

        COUNT_TYPE tmin;
        if (LENGTH % 2 == 0) {
            tmin = (ans[LENGTH / 2] + ans[LENGTH / 2 - 1]) / 2;
        } else {
            tmin = ans[LENGTH / 2];
        }
        tmin = (tmin <= 1) ? 1 : tmin;

        if (ht.find(item) != ht.end()) {
            ++heap[ht[item]].count;
            heap_adjust_down(ht[item]);
        } else if (heap_element_num < capacity) {
            heap[heap_element_num].ID = item;
            heap[heap_element_num].count = tmin;
            ht[item] = heap_element_num;
            ++heap_element_num;
            heap_adjust_up(heap_element_num - 1);
        } else if (tmin > heap[0].count) {
            auto & kv = heap[0];
            ht.erase(kv.ID);
            kv.ID = item;
            kv.count = tmin;
            ht[item] = 0;
            heap_adjust_down(0);
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ans[LENGTH];
        for (int i = 0; i < HASH_NUM; ++i) {
            uint32_t position = hash(item, i) % LENGTH;
            int polar = hash(item, i + HASH_NUM) % 2;
            COUNT_TYPE val = c_sketch[i][position];
            ans[i] = polar ? val : -val;
        }
        Sort(ans, ans + LENGTH);

        COUNT_TYPE tmin;
        if (LENGTH % 2 == 0) {
            tmin = (ans[LENGTH / 2] + ans[LENGTH / 2 - 1]) / 2;
        } else {
            tmin = ans[LENGTH / 2];
        }
        tmin = (tmin <= 0) ? 0 : tmin;
        return tmin;
    }

    void get_top_k_with_frequency(uint16_t k, std::vector<DATA_TYPE, COUNT_TYPE> & result) {
        std::vector<DATA_TYPE, COUNT_TYPE> * a = new std::vector<DATA_TYPE, COUNT_TYPE>[capacity];
        for (int i = 0; i < capacity; ++i) {
            a[i].first = heap[i].ID;
            a[i].second = heap[i].count;
        }
        Sort(a, a + capacity);
        int i;
        for (i = 0; i < k && i < capacity; ++i) {
            result[i].first = a[capacity - 1 - i].first;
            result[i].second = a[capacity - 1 - i].second;
        }
    }

    void get_l2_heavy_hitters(double alpha, std::vector<DATA_TYPE, COUNT_TYPE> & result)
    {
        get_top_k_with_frequency(capacity, result);
        double f2 = get_f2();
        for (int i = 0; i < capacity; ++i) {
            if ((double(result[i].second))*(double(result[i].second)) < alpha * f2) {
                result.resize(i);
                return;
            }
        }
    }

    void get_heavy_hitters(uint32_t threshold, std::vector<std::pair<DATA_TYPE, COUNT_TYPE> >& ret)
    {
        ret.clear();
        for (int i = 0; i < capacity; ++i) {
            if (heap[i].first >= threshold) {
                ret.emplace_back(make_pair(heap[i].ID, heap[i].count));
            }
        }
    }
};

template<typename DATA_TYPE,typename COUNT_TYPE>
class UnivMon : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:
    struct Counter{
        DATA_TYPE ID;
        COUNT_TYPE count;
    };
    uint32_t MEMORY;
    uint32_t LENGTH;
    uint32_t HASH_NUM;
    uint32_t level;
    uint32_t element_num;
    static constexpr uint16_t k = 1000;
    typedef CountHeap<DATA_TYPE, COUNT_TYPE> L2HitterDetector;
    L2HitterDetector ** sketches;
    
    UnivMon(uint32_t _MEMORY, uint32_t _HASH_NUM, std::string _name = "UnivMon"){
        this->name = _name;
        level = 10;
        element_num = 0;
        MEMORY = _MEMORY;
        HASH_NUM = _HASH_NUM;
        LENGTH = MEMORY / HASH_NUM / sizeof(Counter)/ level;
        sketches = new L2HitterDetector*[level];

        for(uint32_t i = 0;i < level; ++i){
            sketches[i] = new L2HitterDetector(MEMORY/10, HASH_NUM, "CountHeap" + std::to_string(i)) ;
        }
    }

    ~UnivMon()
    {
        for (int i = 0; i < level; ++i) {
            delete [] sketches[i];
        }
        delete [] sketches;
    }

    double g_sum(double (*g)(double))
    {
        std::vector<std::pair<DATA_TYPE, COUNT_TYPE>> result(k);
        double Y[level] = {0};

        for (int i = level - 1; i >= 0; i--) {
            sketches[i]->get_top_k_with_frequency(k, result);
            Y[i] = (i == level - 1) ? 0 : 2 * Y[i + 1];
            for (auto & kv: result) {
                if (kv.second == 0) {
                    continue;
                }
                int polar = (i == level - 1) ? 1 : hash(kv.first, 2 * HASH_NUM + i) % 2;
                int coe = (i == level - 1) ? 1 : (1 - 2 * polar);
                Y[i] += coe * g(double(kv.second));
            }
        }

        return Y[0];
    }

    void Insert(DATA_TYPE item)
    {
        int polar;
        element_num++;
        sketches[0]->Insert(item);
        for (int i = 1; i < level; ++i) {
            polar = hash(item, 2 * HASH_NUM + i) % 2;
            if (polar) {
                sketches[i]->Insert(item);
            } else {
                break;
            }
        }
    }

    COUNT_TYPE Query(DATA_TYPE item){
        return sketches[0]->Query(item);
    }

    double get_cardinality()
    {
        return g_sum([](double x) { return 1.0; });
    }

    double get_entropy()
    {
        double sum = g_sum([](double x) { return x == 0 ? 0 : x * std::log2(x); });
        return std::log2(element_num) - sum / element_num;
    }

    void get_heavy_hitters(uint32_t threshold, std::vector<std::pair<DATA_TYPE, COUNT_TYPE> >& ret)
    {
        std::unordered_map<DATA_TYPE, COUNT_TYPE> results;
        std::vector<std::pair<DATA_TYPE, COUNT_TYPE>> vec_top_k(k);
        for (int i = level - 1; i >= 0; --i) {
            sketches[i]->get_top_k_with_frequency(k, vec_top_k);
            for (auto kv: vec_top_k) {
                if (results.find(kv.first) == results.end()) {
                    results[kv.first] = kv.second;
                }
            }
        }

        ret.clear();
        for (auto & kv: results) {
            if (kv.second >= threshold) {
                ret.emplace_back(make_pair(kv.first, kv.second));
            }
        }
    }


};

#endif //CPU_UNIVMON_H