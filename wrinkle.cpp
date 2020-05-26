#if defined(_MSC_VER)
#define NOMINMAX
#endif

#include "wrinkle.hpp"
#include "imageio.hpp"
#include <cassert>
#include <algorithm>

inline bool inRange(Float dist2edge1, Float dist2edge2) {
    return dist2edge1 >= 0 && dist2edge2 >= 0;
}

bool CubicBezier2D::inRange(Point2f p) {
    Vector2f p2firstPoint = p - ctPoints[0];
    Vector2f tangentStart = ctPoints[1] - ctPoints[0];
    Float edge1 = Dot(tangentStart, p2firstPoint);
    if(edge1 < 0)
        return false;
    Vector2f tangentEnd = ctPoints[2] - ctPoints[3];
    Vector2f p2lastPoint = p - ctPoints[3];
    Float edge2 = Dot(tangentEnd, p2lastPoint);
    if(edge2 < 0)
        return false;
    return true;
}

void CubicBezier2D::distance2Edge(Point2f p, Float& edge1, Float& edge2) {
    // is in range?
    Vector2f p2firstPoint = p - ctPoints[0];
    Vector2f tangentStart = ctPoints[1] - ctPoints[0];
    edge1 = Dot(tangentStart, p2firstPoint);
    Vector2f tangentEnd = ctPoints[2] - ctPoints[3];
    Vector2f p2lastPoint = p - ctPoints[3];
    edge2 = Dot(tangentEnd, p2lastPoint);
}

Float CubicBezier2D::roughDistance(Point2f p) {
    //// is in range?
    
    // calc distance to edge
    Vector2f p2firstPoint = p - ctPoints[0];
    Vector2f edge = ctPoints[3] - ctPoints[0];
    Float absin = AbsCross(edge, p2firstPoint);
    Float dist = absin / edge.Length();
    // Float aSqbSq = edge.LengthSquared()*p2firstPoint.LengthSquared();
    // Float abCosTheta = Dot(edge, p2firstPoint);
    // Float a2b2CosTheta2 = abCosTheta * abCosTheta;
    // Float a2b2SinTheta2 = 1-abCosTheta;
    // Float abSinTheta = std::sqrt(a2b2SinTheta2);
    return dist;
}

Float CubicBezier2D::selfWidth() {
    Vector2f middleVectors[2] = {ctPoints[1]-ctPoints[0], ctPoints[2]-ctPoints[0]};
    Vector2f line = ctPoints[3] - ctPoints[0];
    Float sumSin = Cross(line, middleVectors[0]) - Cross(line, middleVectors[1]); // ab1 sin theta1 + ab2 sin theta2
    Float selfWidth = std::abs(sumSin / line.Length());
    return selfWidth;
}

// TODO: remove in_range
Float CubicBezier2D::subdivideAndRecursiveDistance(Point2f p, RangeIndicator range_flag, int depth) {
    bool in_range = range_flag == IN_RANGE;
    static constexpr int MAX_DEPTH = 10;
    bool inDepth = depth <= MAX_DEPTH;
    // subdivide
    Point2f subCtlPoints[7];
    subDivide(subCtlPoints);
    CubicBezier2D subCurve1(subCtlPoints);
    CubicBezier2D subCurve2(subCtlPoints+3);
    Float distanceLeft1, distanceRight1;
    Float distanceLeft2, distanceRight2;
    subCurve1.distance2Edge(p, distanceLeft1, distanceRight1);
    subCurve2.distance2Edge(p, distanceLeft2, distanceRight2);
    bool in_range_local1 = ::inRange(distanceLeft1, distanceRight1);
    bool in_range_local2 = ::inRange(distanceLeft2, distanceRight2);
    Float subdistance1;
    Float subdistance2;
    if ((in_range && in_range_local1)
        || (!in_range && range_flag == LEFT)) {
        subdistance1 = subCurve1.roughDistance(p);
        Float sw1 = subCurve1.selfWidth();
        bool accurateEnough = sw1 == 0.f || subdistance1 / sw1 > ACCURATE_RATIO;
        if (inDepth && !accurateEnough)
            subdistance1 = subCurve1.subdivideAndRecursiveDistance(p, range_flag, depth + 1);
    }
    if ((in_range && in_range_local2)
        || (!in_range && range_flag == RIGHT)) {
        subdistance2 = subCurve2.roughDistance(p);
        Float sw2 = subCurve2.selfWidth();
        bool accurateEnough = sw2 == 0.f || subdistance2 / sw2 > ACCURATE_RATIO;
        if (inDepth && !accurateEnough)
            subdistance2 = subCurve2.subdivideAndRecursiveDistance(p, range_flag, depth + 1);
    }
    // choose the minimum
    if (!in_range) {
        if (range_flag == LEFT)
            return subdistance1;
        else
            return subdistance2;
    }
    else if(in_range_local1 && in_range_local2) {
        return std::min(subdistance1, subdistance2);
    }
    else {
        if(in_range_local1)
            return subdistance1;
        else
            return subdistance2;
    }
}

// two info: in range? distance
Float CubicBezier2D::accurateDistance(Point2f p, RangeIndicator range_flag) {
    Float globalDistance = roughDistance(p);
    // accurate enough?
    Float sw = selfWidth();
    if(sw == 0.f || globalDistance / sw > 100) {
        // accurate enough
        return globalDistance;
    }
    return subdivideAndRecursiveDistance(p, range_flag, 0);
}

static Point3f BlossomBezier(const Point3f p[4], Float u0, Float u1, Float u2) {
    Point3f a[3] = {Lerp(u0, p[0], p[1]), Lerp(u0, p[1], p[2]),
                    Lerp(u0, p[2], p[3])};
    Point3f b[2] = {Lerp(u1, a[0], a[1]), Lerp(u1, a[1], a[2])};
    return Lerp(u2, b[0], b[1]);
}

void CubicBezier2D::subDivide(Point2f* childrenPoints) {
    childrenPoints[0] = ctPoints[0];
    childrenPoints[1] = (ctPoints[0] + ctPoints[1]) / 2;
    childrenPoints[2] = (ctPoints[0] + 2 * ctPoints[1] + ctPoints[2]) / 4;
    childrenPoints[3] = (ctPoints[0] + 3 * ctPoints[1] + 3 * ctPoints[2] + ctPoints[3]) / 8;
    childrenPoints[4] = (ctPoints[1] + 2 * ctPoints[2] + ctPoints[3]) / 4;
    childrenPoints[5] = (ctPoints[2] + ctPoints[3]) / 2;
    childrenPoints[6] = ctPoints[3];
}

Float LargeScaleWrinkle::height(Float distance) {
    // add depth to the original formula.
    Float sum = depth * (distance / width - 1) * std::exp(-distance/width);
    return sum;
}

Float LargeScaleWrinkle::heightP(Point2f p, CubicBezier2D::RangeIndicator range_flag) {
    Float roughDist = curve.roughDistance(p);
    // too far away
    if(roughDist > 10 * width)
        return 0.f;
    Float dist = curve.accurateDistance(p, range_flag);
    return height(dist);
}

Float LargeScaleWrinkle::height(Point2f p) {
    // check distances to 2 edges
    Float edge1, edge2;
    curve.distance2Edge(p, edge1, edge2);
    Float shrinkRatio = 1.0f;
    bool in_range = edge1 >= 0 && edge2 >= 0;
    CubicBezier2D::RangeIndicator range_flag = CubicBezier2D::RangeIndicator::IN_RANGE;
    if(!in_range) {
        // test distance to the edge
        Float distance;
        if (edge1 < 0) {
            distance = -edge1;
            range_flag = CubicBezier2D::RangeIndicator::LEFT;
        }
        else {
            distance = -edge2;
            range_flag = CubicBezier2D::RangeIndicator::RIGHT;
        }
        // very far away?
        if(distance > 3 * width)
            return 0;
        shrinkRatio = 1.0f - distance / 3.0 / width;
    }     
    return shrinkRatio * heightP(p, range_flag);
}

void Canvas::WriteWrinkles() {
    Float ratio = 1.0 / (Float)map.size();
    for(int i=0;i<map.size();i++) {
        for(int j=0;j<map.size();j++) {
            Point2f worldPos = { i * ratio * world_size, j * ratio * world_size };
            map.Set(i, j, 0.0);
            for(auto& w : wrinkles) {
                Float currHeight = w.height(worldPos);
                map.Add(i, j, currHeight);
                if (currHeight > maxHeight)
                    maxHeight = currHeight;
                if (currHeight < minHeight)
                    minHeight = currHeight;
            }
        }
    }
    LOG(INFO) << "maxHeight:" << maxHeight;
    LOG(INFO) << "minHeight:" << minHeight;
}

void Canvas::WritePNG() {
    std::vector<unsigned char> buffer;
    buffer.resize(map.size()*map.size());
    Float* d = map.data();
    for (int i = 0; i < map.size() * map.size(); i++) {
        Float normHeight = (*d++ - minHeight) / (maxHeight - minHeight);
        buffer[i] = static_cast<unsigned char>(normHeight * 255);
    }
    //WritePNGfromChar
    WritePNGfromChar("wrinkle.png", (char*)buffer.data(), map.size(), map.size(), 1);
}

void Canvas::WriteTIFF() {
    // make all above zero
    Float* d = map.data();
    for (int i = 0; i < map.size() * map.size(); i++) {
        *d -= minHeight;
        d++;
    }

    WriteTIFFfromFloat("wrinkle.tiff", map.data(), map.size(), map.size(), 1, sizeof(float) * 8);
}