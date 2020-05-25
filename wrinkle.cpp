#if defined(_MSC_VER)
#define NOMINMAX
#endif

#include "wrinkle.hpp"
#include "imageio.hpp"
#include <cassert>
#include <algorithm>

Float CubicBezier2D::roughDistance(Point2f p, bool* in_range) {
    // is in range?
    Vector2f p2firstPoint = p - ctPoints[0];
    Vector2f tangentStart = ctPoints[1] - ctPoints[0];
    Float edge1 = Dot(tangentStart, p2firstPoint);
    if(edge1<0) {
        *in_range = false;
        return 0.f;
    }
    Vector2f tangentEnd = ctPoints[2] - ctPoints[3];
    Vector2f p2lastPoint = p - ctPoints[3];
    Float edge2 = Dot(tangentEnd, p2lastPoint);
    if(edge2<0) {
        *in_range = false;
        return 0.f;
    }
    *in_range = true;
    // calc distance to edge
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

Float CubicBezier2D::subdivideAndRecursiveDistance(Point2f p, bool* in_range, int depth) {
    static constexpr int MAX_DEPTH = 10;
    bool inDepth = depth <= MAX_DEPTH;
    // subdivide
    Point2f subCtlPoints[7];
    subDivide(subCtlPoints);
    CubicBezier2D subCurve1(subCtlPoints);
    CubicBezier2D subCurve2(subCtlPoints+3);
    // test each
    bool in_range_local1 = false;
    Float subdistance1 = subCurve1.roughDistance(p, &in_range_local1);
    Float sw1 = subCurve1.selfWidth();
    if(in_range_local1 && inDepth) {
        if(sw1 == 0.f || subdistance1 / sw1 > 100)
            ; // accurate enough
        else
            subdistance1 = subCurve1.subdivideAndRecursiveDistance(p, &in_range_local1, depth+1);
    }
    bool in_range_local2 = false;
    Float subdistance2 = subCurve2.roughDistance(p, &in_range_local2);
    Float sw2 = subCurve2.selfWidth();
    if(in_range_local2 && inDepth) {
        if(sw2 == 0.f || subdistance2 / sw2 > 100)
            ; // accurate enough
        else
            subdistance2 = subCurve2.subdivideAndRecursiveDistance(p, &in_range_local2, depth+1);
    }
    // choose the minimum
    if(in_range_local1 && in_range_local2) {
        *in_range = true;
        return std::min(subdistance1, subdistance2);
    }
    else if(!in_range_local1 && !in_range_local2) {
        assert(0 && "In range in caller function. Out of range in callee");
    }
    else {
        *in_range = true;
        if(in_range_local1)
            return subdistance1;
        else
            return subdistance2;
    }
}

// two info: in range? distance
Float CubicBezier2D::distance(Point2f p, bool* in_range) {
    Float globalDistance = roughDistance(p, in_range);
    if(!*in_range)
        return 0.f;
    // accurate enough?
    Float sw = selfWidth();
    if(sw == 0.f || globalDistance / sw > 100) {
        // accurate enough
        return globalDistance;
    }
    return subdivideAndRecursiveDistance(p, in_range, 0);
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

Float LargeScaleWrinkle::height(Point2f p) {
    // if too far, 
    bool in_range;
    Float roughDist = curve.roughDistance(p, &in_range);
    if(!in_range || roughDist > 10 * width)
        return 0.f;
    Float dist = curve.distance(p, &in_range);
    return height(dist);
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