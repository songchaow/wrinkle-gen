#include <vector>
#include "wrinkle.hpp"


struct ForeheadWrinkleLayout {
    struct ForeheadWrinkleConfig {
        // configs
        uint32_t numWrinkleLines_min;
        uint32_t numWrinkleLines_max;
        uint32_t numSegmentPerLine_min;
        uint32_t numSegmentPerLine_max;

        


        // Generated hyper parameters
        uint32_t numWrinkleLines; // visible wrinkle number each line
        uint32_t numCurvePerLine; // number of cubic bezier curves, same in each line
        std::vector<std::vector<int>> numSegmentPerLine;
    };
    ForeheadWrinkleConfig config;
    //std::vector<>
    void SpawnOneWrinkle() {

    }

};