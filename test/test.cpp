#include "wrinkle.hpp"

int main() {
    // control points
    Point2f ctlPoints[] = {
        {0.0, 0.0},
        {1.0, 1.0},
        {2.0, -1.0},
        {3.0, 0.0}
    };
    CubicBezier2D curve(ctlPoints);
    LargeScaleWrinkle wrinkle{
        curve,
        1.0, // depth
        0.1, // width
    };
    Canvas canvas(1024, 3);
    canvas.AddWrinkle(wrinkle);
    canvas.WriteWrinkles();
    canvas.WritePNG();
    return 0;
}