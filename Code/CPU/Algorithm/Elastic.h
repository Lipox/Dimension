#ifndef CPU_ELASTIC_H
#define CPU_ELASTIC_H

#include <cmath>
#include "Abstract.h"

#define COUNTER_PER_BUCKET 4
#define LAMBDA 8
#define HEAVY_PART_LENGTH 250000
#define LIGHT_PART_LENGTH 1000000

template<typename DATA_TYPE,typename HEAVY_TYPE>
class HeavyPart {
public:

	HeavyPart(){
		buckets = new Bucket[HEAVY_PART_LENGTH];
		clear();
	}

	~HeavyPart() {
		delete[] buckets;
	}

	void clear()
	{
		memset(buckets, 0, sizeof(Bucket) * HEAVY_PART_LENGTH);
	}

	uint32_t insert(const DATA_TYPE key, HEAVY_TYPE& swap_val) {
		uint32_t position = hash(key) % HEAVY_PART_LENGTH;
		
		int matched = -1, empty = -1, min_counter_key = 0, min_counter_val = INT_MAX;

		for (uint32_t i = 0; i < COUNTER_PER_BUCKET; i++)
		{
			if (buckets[position].keys[i] == key)
			{
				matched = i;
				break;
			}
			if (buckets[position].keys[i] == 0 && empty == -1)
			{
				empty = i;
			}
			if (buckets[position].values[i] < min_counter_val)
			{
				min_counter_key = i;
				min_counter_val = buckets[position].values[i];
			}
		}
		//printf("matched = %d,empty = %d, min_counter_val = %d \n", matched, empty, min_counter_val);

		/* if matched */
		if (matched != -1) {
			buckets[position].values[matched] += 1;
			return 0;
		}

		/* if there has empty bucket */
		if (empty != -1) {
			buckets[position].keys[empty] = key;
			buckets[position].values[empty] = 1;
			return 0;
		}

		/* no matched */
		if ((buckets[position].minus_vote + 1) >= min_counter_val * LAMBDA)
		{
			buckets[position].minus_vote = 0;
			buckets[position].flags[min_counter_key] = 1;
			swap_val = buckets[position].values[min_counter_key];
			buckets[position].keys[min_counter_key] = key;
			buckets[position].values[min_counter_key] = 1;

			return 2;
		}
		else {
			buckets[position].minus_vote += 1;
			return 1;
		}
	}

	HEAVY_TYPE query(const DATA_TYPE key, uint8_t& flag) {
		uint32_t position = hash(key) % HEAVY_PART_LENGTH;
		
		for (size_t i = 0; i < COUNTER_PER_BUCKET; i++)
		{
			if (buckets[position].keys[i] == key)
			{
				flag = buckets[position].flags[i];
				return buckets[position].values[i];
			}
		}

		return 0;
	}

private:
	struct Bucket
	{
		DATA_TYPE keys[COUNTER_PER_BUCKET];
		HEAVY_TYPE values[COUNTER_PER_BUCKET];
		uint8_t flags[COUNTER_PER_BUCKET];
		HEAVY_TYPE minus_vote;
	};

	Bucket* buckets;
};

template<typename DATA_TYPE,typename LIGHT_TYPE>
class LightPart {
public:

	LightPart(){
		counters = new LIGHT_TYPE[LIGHT_PART_LENGTH];
		clear();
	}

	~LightPart() {
		delete[] counters;
	}

	void clear()
	{
		memset(counters, 0, sizeof(LIGHT_TYPE) * LIGHT_PART_LENGTH);
	}

	void insert(const DATA_TYPE key, LIGHT_TYPE f = 1) {
		uint32_t position = hash(key) % LIGHT_PART_LENGTH;
		LIGHT_TYPE max_num = (LIGHT_TYPE)pow(2, sizeof(LIGHT_TYPE) * 8) - 1;

		LIGHT_TYPE old_val = counters[position];
		LIGHT_TYPE new_val = counters[position] + f;

		new_val = new_val > max_num ? max_num : new_val;

		counters[position] = new_val;
	}

	LIGHT_TYPE query(const DATA_TYPE key) {
		uint32_t position = hash(key) % LIGHT_PART_LENGTH;
		return counters[position];
	}

private:
	LIGHT_TYPE* counters;
};

template<typename DATA_TYPE, typename COUNT_TYPE>
class Elastic : public Abstract<DATA_TYPE, COUNT_TYPE> {
	//typedef LIGHT_TYPE COUNT_TYPE;
public:
	Elastic(){}

	~Elastic(){}

	void clear()
	{
		heavy_part.clear();
		light_part.clear();
	}

	void Insert(const DATA_TYPE item) {
		COUNT_TYPE swap_val = 0;
		uint32_t result = heavy_part.insert(item, swap_val);

		switch (result){
			case 0: return;
			case 1: {
				light_part.insert(item);
				return;
			}
			case 2: {
				light_part.insert(item, swap_val);
				return;
			}
			default: {
				printf("error return value !\n");
				exit(1);
			}
		}
	}

	COUNT_TYPE Query(const DATA_TYPE item) {
		uint8_t flag = 0;
		COUNT_TYPE heavy_result = heavy_part.query(item, flag);
		if (heavy_result == 0 || flag == 1) {
			COUNT_TYPE light_result = light_part.query(item);
			return heavy_result + light_result;
		}
		return heavy_result;
	}

private:
	HeavyPart<DATA_TYPE, COUNT_TYPE> heavy_part;
	LightPart<DATA_TYPE, COUNT_TYPE> light_part;

};

#endif //CPU_ELASTIC_H
