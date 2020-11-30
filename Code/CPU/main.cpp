#include "SketchBench.h"
#include "TrafficBench.h"

const char* file[] = {
        "/Users/zyd/Documents/Data/ip.dat"
        //in server /zhangyd/Data/ip.dat
};

int main() {
    for(uint32_t i = 0;i < 1;++i){
        std::cout << file[i] << std::endl;
        SketchBench<uint64_t, int64_t> sketchBench(file[i]);
        sketchBench.FEBench(10000000);
        sketchBench.HHBench(500000, 0.0001);

        TrafficBench<uint64_t, int64_t> trafficBench(file[i]);
        trafficBench.FEBench(20000000, 4);
        trafficBench.HHBench(2000000, 4, 0.0001);
    }
    return 0;
}