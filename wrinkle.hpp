#pragma once

#include "geometry.hpp"
#include "heightmap.hpp"
#include <vector>

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

struct LargeScaleWrinkle {
    CubicBezier2D curve;
    Float depth; // param d
    Float width; // param w
    Float height(Float distance); // S(l)
    Float height(Point2f p);
};

class Canvas {
    HeightMap map;
    Float world_size;
    std::vector<LargeScaleWrinkle> wrinkles;
public:
    Canvas(uint32_t resolution, uint32_t world_size) : map(resolution), world_size(world_size) {}
    void AddWrinkle(const LargeScaleWrinkle& w) { wrinkles.push_back(w); }
    void WriteWrinkles();
    void WritePNG();
};