#pragma once

#include "geometry.hpp"
#include "heightmap.hpp"
#include <vector>
#include <cmath>
#include <cstring>

struct CubicBezier2D {
public:
    enum RangeIndicator {
        IN_RANGE,
        LEFT,
        RIGHT
    };
protected:
    Float subdivideAndRecursiveDistance(Point2f p, RangeIndicator range_flag, int depth);
    void subDivide(Point2f* childrenPoints);
    Float selfWidth();
public:
    
    static constexpr int NUM_POINT = 4;
    static constexpr int ACCURATE_RATIO = 100;
    Point2f ctPoints[NUM_POINT];
    Float roughDistance(Point2f p);
    Float accurateDistance(Point2f p, RangeIndicator range_flag);
    void distance2Edge(Point2f p, Float& edge1, Float& edge2);
    bool inRange(Point2f p);
    CubicBezier2D(Point2f* dataStart) {
        std::memcpy(ctPoints, dataStart, sizeof(Point2f)*NUM_POINT);
    }
};

struct LargeScaleWrinkle {
    CubicBezier2D curve;
    Float depth; // param d, unit is cm in reality
    Float width; // param w
    Float maxHeight;
    // height purely affected by the perpendicular distance
    Float heightP(Point2f p, CubicBezier2D::RangeIndicator range_flag);
    Float height(Float distance); // S(l)
    Float height(Point2f p);
    LargeScaleWrinkle(CubicBezier2D curve, Float depth, Float width) : curve(curve), depth(depth), width(width),
        maxHeight(depth*(1.f + std::exp(-2.f))) {}
};

class Canvas {
    HeightMap map;
    Float world_size;
    std::vector<LargeScaleWrinkle> wrinkles;
    Float maxHeight = 0.0;
    Float minHeight = 0.0;
public:
    Canvas(uint32_t resolution, Float world_size) : map(resolution), world_size(world_size) {}
    void AddWrinkle(const LargeScaleWrinkle& w) { wrinkles.push_back(w); }
    void WriteWrinkles();
    void WritePNG();
    void WriteTIFF();
};
