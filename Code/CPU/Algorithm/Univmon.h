#ifndef CPU_UNIVMON_H
#define CPU_UNIVMON_H

#include "../Common/hash.h"

using std::min;
using std::swap;

#define SQR(X) (X) * (X)

template<uint8_t key_len, int capacity, int d = 3>  //capacity是堆的元素上限，d是哈希函数个数
struct CountHeap {
public:
    typedef pair <string, int> KV;
    typedef pair <int, string> VK;
    VK heap[capacity];  //记录出现次数最多的元素及他出现的次数
    int heap_element_num;  //堆中元素个数
    int mem_in_bytes;  //sketch的内存（单位：Bytes）
    int w;  //哈希范围
    int * cm_sketch[d];
    BOBHash32 * hash[d];  
    BOBHash32 * hash_polar[d];
    unordered_map<string, uint32_t> ht;  //记录元素在堆中的位置
    string name;

    double get_f2() //返回sketch中记录的平方和的中位数
    {
        double res[d];
        for (int i = 0; i < d; ++i) {
            double est = 0;
            for (int j = 0; j < w; ++j) {
                est += SQR(double(cm_sketch[i][j]));
            }
            res[i] = est;
        }

        sort(res, res + d);
        if (d % 2) {
            return res[d / 2];
        } else {
            return (res[d / 2] + res[d / 2 - 1]) / 2;
        }
    }

    void heap_adjust_down(int i) {  //调整元素在堆中位置
        while (i < heap_element_num / 2) {
            int l_child = 2 * i + 1;
            int r_child = 2 * i + 2;
            int larger_one = i;
            if (l_child < heap_element_num && heap[l_child] < heap[larger_one]) {
                larger_one = l_child;
            }
            if (r_child < heap_element_num && heap[r_child] < heap[larger_one]) {
                larger_one = r_child;
            }
            if (larger_one != i) {
                swap(heap[i], heap[larger_one]);
                swap(ht[heap[i].second], ht[heap[larger_one].second]);
                heap_adjust_down(larger_one);
            } else {
                break;
            }
        }
    }

    void heap_adjust_up(int i) {  //调整元素在堆中位置
        while (i > 1) {
            int parent = (i - 1) / 2;
            if (heap[parent] <= heap[i]) {
                break;
            }
            swap(heap[i], heap[parent]);
            swap(ht[heap[i].second], ht[heap[parent].second]);
            i = parent;
        }
    }

    CountHeap(int mem_in_bytes_) : mem_in_bytes(mem_in_bytes_), heap_element_num(0) {
		w = mem_in_bytes / 4 / d;
        for (int i = 0; i < capacity; ++i) {
            heap[i].first = 0;
        }
        memset(cm_sketch, 0, sizeof(cm_sketch));
        srand(time(0));
        for (int i = 0; i < d; i++) {
            hash[i] = new BOBHash32(uint32_t(rand() % MAX_PRIME32));
            hash_polar[i] = new BOBHash32(uint32_t(rand() % MAX_PRIME32));
            cm_sketch[i] = new int[w];
            memset(cm_sketch[i], 0, sizeof(int)*w);
        }

        stringstream name_buf;
        name_buf << "CountHeap@" << mem_in_bytes;
        name = name_buf.str();
    }

    void insert(uint8_t * key) {
        int ans[d];

        for (int i = 0; i < d; ++i) {
            int idx = hash[i]->run((char *)key, key_len) % w;
            int polar = hash_polar[i]->run((char *)key, key_len) % 2;
            cm_sketch[i][idx] += polar ? 1 : -1;
            int val = cm_sketch[i][idx];
            ans[i] = polar ? val : -val;
        }
        sort(ans, ans + d);

        int tmin;
        if (d % 2 == 0) {
            tmin = (ans[d / 2] + ans[d / 2 - 1]) / 2;
        } else {
            tmin = ans[d / 2];
        }
        tmin = (tmin <= 1) ? 1 : tmin;

        string str_key = string((const char *)key, key_len);
        if (ht.find(str_key) != ht.end()) {
            heap[ht[str_key]].first++;
            heap_adjust_down(ht[str_key]);
        } else if (heap_element_num < capacity) {
            heap[heap_element_num].second = str_key;
            heap[heap_element_num].first = tmin;
            ht[str_key] = heap_element_num++;
            heap_adjust_up(heap_element_num - 1);
        } else if (tmin > heap[0].first) {
            VK & kv = heap[0];
            ht.erase(kv.second);
            kv.second = str_key;
            kv.first = tmin;
            ht[str_key] = 0;
            heap_adjust_down(0);
        }
    }

    void get_top_k_with_frequency(uint16_t k, vector<KV> & result) {
        VK * a = new VK[capacity];
        for (int i = 0; i < capacity; ++i) {
            a[i] = heap[i];
        }
        sort(a, a + capacity);
        int i;
        for (i = 0; i < k && i < capacity; ++i) {
            result[i].first = a[capacity - 1 - i].second;
            result[i].second = a[capacity - 1 - i].first;
        }
    }

    void get_l2_heavy_hitters(double alpha, vector<KV> & result)
    {
        get_top_k_with_frequency(capacity, result);
        double f2 = get_f2();
        for (int i = 0; i < capacity; ++i) {
            if (SQR(double(result[i].second)) < alpha * f2) {
                result.resize(i);
                return;
            }
        }
    }

    void get_heavy_hitters(uint32_t threshold, std::vector<pair<string, uint32_t> >& ret)
    {
        ret.clear();
        for (int i = 0; i < capacity; ++i) {
            if (heap[i].first >= threshold) {
                ret.emplace_back(make_pair(heap[i].second, heap[i].first));
            }
        }
    }

    ~CountHeap() {
        for (int i = 0; i < d; ++i) {
            delete hash[i];
            delete hash_polar[i];
            delete cm_sketch[i];
        }
        return;
    }
};

template<uint8_t key_len, uint64_t mem_in_bytes, uint8_t level = 14>
class UnivMon
{
public:
    static constexpr uint16_t k = 1000;
    typedef CountHeap<key_len, k, 5> L2HitterDetector;
    L2HitterDetector * sketches[level];
    BOBHash32 * polar_hash[level];
    int element_num = 0;

    double g_sum(double (*g)(double))
    {
        std::vector<pair<std::string, int>> result(k);
        double Y[level] = {0};

        for (int i = level - 1; i >= 0; i--) {
            sketches[i]->get_top_k_with_frequency(k, result);
            Y[i] = (i == level - 1) ? 0 : 2 * Y[i + 1];
            for (auto & kv: result) {
                if (kv.second == 0) {
                    continue;
                }
                int polar = (i == level - 1) ? 1 : (polar_hash[i + 1]->run(kv.first.c_str(), key_len)) % 2;
                int coe = (i == level - 1) ? 1 : (1 - 2 * polar);
                Y[i] += coe * g(double(kv.second));
            }
        }

        return Y[0];
    }
    string name;

    UnivMon()
    {
        stringstream name_buffer;
        name_buffer << "UnivMon@" << mem_in_bytes;
        name = name_buffer.str();
        double total = (1u << level) - 1;
        for (int i = 0; i < level; ++i) {
            int mem_for_sk = int(mem_in_bytes) - level * (key_len + 4) * k;
            int mem = int(mem_for_sk / level);
            sketches[i] = new L2HitterDetector(mem);
            auto idx = uint32_t(rand() % MAX_PRIME32);
            polar_hash[i] = new BOBHash32(idx);
        }
    }

    void insert(uint8_t * key)
    {
        int polar;
        element_num++;
        sketches[0]->insert(key);
        for (int i = 1; i < level; ++i) {
            polar = ((polar_hash[i]->run((const char *)key, key_len))) % 2;
            if (polar) {
                sketches[i]->insert(key);
            } else {
                break;
            }
        }
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

    void get_heavy_hitters(uint32_t threshold, std::vector<pair<uint32_t, int> >& ret)
    {
        unordered_map<std::string, uint32_t> results;
        vector<std::pair<std::string, int>> vec_top_k(k);
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
                ret.emplace_back(make_pair(*(uint32_t *)(kv.first.c_str()), kv.second));
            }
        }
    }

    ~UnivMon()
    {
        for (int i = 0; i < level; ++i) {
            delete sketches[i];
            delete polar_hash[i];
        }
    }
};
#endif //CPU_UNIVMON_H
