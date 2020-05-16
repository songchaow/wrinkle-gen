#pragma once

#include "geometry.hpp"

struct CubicBezier2D {
protected:
    Float subdivideAndRecursiveDistance(Point2f p, bool* in_range, int depth);
    void subDivide(Point2f* childrenPoints);
    Float selfWidth();
public:
    static constexpr int NUM_POINT = 4;
    Point2f ctPoints[NUM_POINT];
    Float roughDistance(Point2f p, bool* in_range);
    Float distance(Point2f p, bool* in_range);
    CubicBezier2D(Point2f* dataStart) {
        std::memcpy(ctPoints, dataStart, sizeof(Point2f)*NUM_POINT);
    }
};

class LargeScaleWrinkle {
    CubicBezier2D curve;
};