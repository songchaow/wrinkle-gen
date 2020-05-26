#include "wrinkle.hpp"

int main() {
    // control points
    Point2f ctlPoints[] = {
        {0.0, 1.0},
        {1.0, 2.0},
        {2.0, 0.0},
        {2.5, 1.0}
    };
    CubicBezier2D curve(ctlPoints);
    LargeScaleWrinkle wrinkle(curve, 1.0f, 0.1f);
    Canvas canvas(1024, 3);
    canvas.AddWrinkle(wrinkle);
    canvas.WriteWrinkles();
    //canvas.WritePNG();
    canvas.WriteTIFF();
    return 0;
}