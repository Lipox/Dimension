#include "benchmark.h"

const char* file[] = {
        "/Users/zyd/Documents/Data/ip.dat"
        //in server /zhangyd/Data/ip.dat
};

int main() {
    for(uint32_t i = 0;i < 1;++i){
        std::cout << file[i] << std::endl;
        BenchMark<uint64_t, int64_t> dataset(file[i]);
        dataset.Parameter(0.0002);
    }
    return 0;
}