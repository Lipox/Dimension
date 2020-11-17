#ifndef CPU_ELASTIC_H
#define CPU_ELASTIC_H

#include <cmath>
#include "Abstract.h"

#define COUNTER_PER_BUCKET 7
#define LAMBDA 8
#define MAXNUM 0xff


template<typename DATA_TYPE, typename COUNT_TYPE>
class Elastic : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    typedef uint8_t LIGHT_TYPE;

    struct Bucket{
        COUNT_TYPE vote;
        uint8_t flags[COUNTER_PER_BUCKET];
        DATA_TYPE ID[COUNTER_PER_BUCKET];
        COUNT_TYPE count[COUNTER_PER_BUCKET];

        COUNT_TYPE Query(const DATA_TYPE item, uint8_t& flag) {
            for(uint32_t i = 0; i < COUNTER_PER_BUCKET; i++) {
                if(ID[i] == item) {
                    flag = flags[i];
                    return count[i];
                }
            }
            return 0;
        }
    };

	Elastic(uint32_t _MEMORY, std::string _name = "Elastic"){
	    this->name = _name;

	    HEAVY_LENGTH = _MEMORY * HEAVY_RATIO / sizeof(Bucket);
        LIGHT_LENGTH = _MEMORY * LIGHT_RATIO / sizeof(LIGHT_TYPE);

        buckets = new Bucket[HEAVY_LENGTH];
        counters = new LIGHT_TYPE[LIGHT_LENGTH];

        memset(buckets, 0, sizeof(Bucket) * HEAVY_LENGTH);
        memset(counters, 0, sizeof(LIGHT_TYPE) * LIGHT_LENGTH);
	}

	~Elastic(){
        delete [] counters;
        delete [] buckets;
	}

	void Insert(const DATA_TYPE item) {
        uint32_t pos = hash(item) % HEAVY_LENGTH, minPos = 0;
        COUNT_TYPE minVal = 0;

        for (uint32_t i = 0; i < COUNTER_PER_BUCKET; i++){
            if(buckets[pos].ID[i] == item){
                buckets[pos].count[i] += 1;
                return;
            }

            if(buckets[pos].count[i] == 0){
                buckets[pos].ID[i] = item;
                buckets[pos].count[i] = 1;
                return;
            }

            if(buckets[pos].count[i] < minVal){
                minPos = i;
                minVal = buckets[pos].count[i];
            }
        }

        if((buckets[pos].vote + 1) >= minVal * LAMBDA){
            buckets[pos].vote = 0;
            buckets[pos].flags[minPos] = 1;

            Light_Insert(buckets[pos].ID[minPos], buckets[pos].count[minPos]);

            buckets[pos].ID[minPos] = item;
            buckets[pos].count[minPos] = 1;
        }
        else {
            buckets[pos].vote += 1;
            Light_Insert(item);
        }
	}

	COUNT_TYPE Query(const DATA_TYPE item) {
		uint8_t flag = 1;
		COUNT_TYPE result = buckets[hash(item) % HEAVY_LENGTH].Query(item, flag);
		if(flag){
		    return result + counters[hash(item, 101) % LIGHT_LENGTH];
		}
		else{
		    return result;
		}
	}

private:

    const double HEAVY_RATIO = 0.25;
    const double LIGHT_RATIO = 0.75;

    uint32_t LIGHT_LENGTH;
    uint32_t HEAVY_LENGTH;

    LIGHT_TYPE* counters;
    Bucket* buckets;

    void Light_Insert(const DATA_TYPE item, COUNT_TYPE val = 1) {
        uint32_t position = hash(item, 101) % LIGHT_LENGTH;

        COUNT_TYPE old_val = counters[position];
        COUNT_TYPE new_val = counters[position] + val;

        counters[position] = (new_val > MAXNUM ? MAXNUM : new_val);
    }
};

#endif //CPU_ELASTIC_H
