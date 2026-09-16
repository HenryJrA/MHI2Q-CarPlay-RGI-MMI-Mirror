#ifndef VIDEO_GEOMETRY_H
#define VIDEO_GEOMETRY_H

/* Output pixels use a top-left origin. UVs use the captured frame's existing
 * top-to-bottom orientation: (0,0) is its first pixel, (1,1) its opposite edge. */
struct VideoGeometry {
    int x;
    int y;
    int width;
    int height;
    float u0;
    float v0;
    float u1;
    float v1;

    VideoGeometry()
        : x(0), y(0), width(0), height(0),
          u0(0.0f), v0(0.0f), u1(1.0f), v1(1.0f) {}

    bool valid_for(int output_width, int output_height) const {
        /* Subtraction avoids overflowing x+width/y+height on invalid input.
         * Ordered comparisons also reject NaN/infinite UV coordinates. */
        return output_width > 0 && output_height > 0 &&
            x >= 0 && y >= 0 && x <= output_width && y <= output_height &&
            width > 0 && height > 0 &&
            width <= output_width - x && height <= output_height - y &&
            u0 >= 0.0f && v0 >= 0.0f && u1 <= 1.0f && v1 <= 1.0f &&
            u0 < u1 && v0 < v1;
    }
};

#endif
