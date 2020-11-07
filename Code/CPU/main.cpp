#include "benchmark.h"

const char* file[] = {
        "../../Data/syn.dat",
        "../../Data/web.dat",
        "../../Data/net.dat",
//        "../../Data/ip.dat",
};

int main() {

    for(uint32_t i = 0;i < 3;++i){
        std::cout << file[i] << std::endl;
        BenchMark<uint32_t, int32_t> dataset(file[i]);
        dataset.SketchError(1);
        dataset.SketchError(2);
        dataset.SketchError(4);
        dataset.SketchError(8);
        dataset.SketchError(16);
    }

    return 0;
}