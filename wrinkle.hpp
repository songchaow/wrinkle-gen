#pragma once

#include "geometry.hpp"
#include "heightmap.hpp"
#include "ext/spline.h"

#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>

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
    Float roughProgress(Point2f p);
    void subdivideAndRecursiveProgress(Point2f p, RangeIndicator range_flag, int depth,
    Float lastProgess, float startingProgress, std::vector<std::pair<float,float>>& perpendicular_locs);
    Float accurateDistance(Point2f p, RangeIndicator range_flag);
    // returns a paramter between 0 and 1
    //Float accurateProgress(Point2f p);
    void distance2Edge(Point2f p, Float& edge1, Float& edge2);
    void distance2FirstEdge(Point2f p, Float& edge1);
    bool inRange(Point2f p);
    CubicBezier2D(Point2f* dataStart) {
        std::memcpy(ctPoints, dataStart, sizeof(Point2f)*NUM_POINT);
    }
    Vector2f Dir() { return ctPoints[NUM_POINT-1] - ctPoints[0];}
};

class HeightLine {
    static constexpr uint32_t numSamplePoints_default = 5;
    std::vector<double> numSampleHeights;
    std::vector<float> numSampleMasks; // visually seen num of lines

    tk::spline spline;
protected:
    void Interpolate() {
        std::vector<double> xs;
        double delta = 1.0 / (numSampleHeights.size() - 1);
        double currProgress = 0.0;
        for(int i = 0; i < numSampleHeights.size(); i++) {
            xs.push_back(currProgress);
            currProgress += delta;
        }
        spline.set_points(xs, numSampleHeights);
    }
public:
    HeightLine(float maxHeight = 1.0f, uint32_t numSamplePoints = numSamplePoints_default)
        : numSampleHeights(numSamplePoints), numSampleMasks(numSamplePoints) {
        numSampleHeights.reserve(numSamplePoints);
        numSampleMasks.reserve(numSamplePoints); // ?
        // random init
        numSampleHeights.push_back(0.f);
        for(int i = 1; i < numSamplePoints - 1; i++) {
            float randHeight = (float)std::rand() / RAND_MAX * maxHeight;
            numSampleHeights.push_back(randHeight);
        }
        numSampleHeights.push_back(0.f);

        Interpolate();
    }

    float height(float progress) {
        return spline(progress);
    }
};

struct LargeScaleWrinkle {
    CubicBezier2D curve;
    Float depth; // param d, unit is cm in reality
    Float width; // param w, actual width: 4w
    Float maxHeight;
    // height purely affected by the perpendicular distance
    Float heightP(Point2f p, CubicBezier2D::RangeIndicator range_flag);
    Float height(Float distance); // S(l)
    Float height(Point2f p);
    LargeScaleWrinkle(CubicBezier2D curve, Float depth, Float width) : curve(curve), depth(depth), width(width),
        maxHeight(depth*(1.f + std::exp(-2.f))) {}
};

struct BezierSegment2D {
    Point2f spawningPoint;
    std::vector<CubicBezier2D> segments;
    BezierSegment2D(Point2f start) : spawningPoint(start) {}
    // newdir should be normalized
    void SpawnWithDir(Vector2f newdir, float length, float maxHeight) {
        Point2f points[4];
        Vector2f dirP = newdir.Perpendicular();
        if(segments.size()==0) {
            points[0] = spawningPoint;
            // point[1]
            float basicLen = length / 3;

            float randratio = (float)std::rand() / RAND_MAX - 0.5f;
            float randratio_height = (float)std::rand() * 2.f / RAND_MAX - 1.f;

            float height = maxHeight * randratio_height;
            basicLen += randratio * length / 6;
            
            points[1] = points[0] + basicLen * newdir + height * dirP;
        }
        else {
            points[0] = segments.back().ctPoints[3];
            // point[1], set according to point[2] in last segment
            Vector2f offset = segments.back().ctPoints[3] - segments.back().ctPoints[2];
            points[1] = points[0] + offset;
        }
        // point[2]
        float basicLen = length / 3 * 2;
        float randratio = (float)std::rand() / RAND_MAX - 0.5f;
        basicLen += randratio * length / 6;

        float randratio_height = (float)std::rand() / RAND_MAX * 2.f - 1.f;
        float height = maxHeight * randratio_height;

        points[2] = points[0] + basicLen * newdir + height * dirP;

        // points[3], along the direction
        points[3] = points[0] + length * newdir;
        segments.emplace_back(points);
    }
    std::vector<CubicBezier2D>::iterator begin() { return segments.begin(); }
    std::vector<CubicBezier2D>::iterator end() { return segments.end(); }
};

class MultiSegmentWrinkle {
    BezierSegment2D ms_wrinkle;
    HeightLine heightline;
    Float width;
public:
    MultiSegmentWrinkle(BezierSegment2D segment, float width) : ms_wrinkle(segment), width(width) {}
    float height(Point2f p);
    float heightPerpendicular(float distance) {
        Float sum = (distance / width - 1) * std::exp(-distance/width);
        return sum;
    }
};

class Canvas {
    HeightMap map;
    Float world_size;
    std::vector<LargeScaleWrinkle> wrinkles;
    std::vector<MultiSegmentWrinkle> ms_wrinkles;
    Float maxHeight = 0.0;
    Float minHeight = 0.0;
public:
    Canvas(uint32_t resolution, Float world_size) : map(resolution), world_size(world_size) {}
    void AddWrinkle(const LargeScaleWrinkle& w) { wrinkles.push_back(w); }
    void AddMultiSegWrinkle(const MultiSegmentWrinkle& w) { ms_wrinkles.push_back(w); }
    void WriteWrinkles();
    void WritePNG();
    void WriteTIFF();
};
