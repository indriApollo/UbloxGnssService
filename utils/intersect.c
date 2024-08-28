//
// Created by rleroux on 8/28/24.
//

// https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/

#include <stdint.h>
#include <stdbool.h>
#include "intersect.h"

#define COLLINEAR 0
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 2

int32_t max(int32_t a, int32_t b) {
    return a > b ? a : b;
}

int32_t min(int32_t a, int32_t b) {
    return a < b ? a : b;
}

bool on_segment(coord p, coord q, coord r) {
    if (q.lon <= max(p.lon, r.lon) && q.lon >= min(p.lon, r.lon) &&
        q.lat <= max(p.lat, r.lat) && q.lat >= min(p.lat, r.lat))
        return true;

    return false;
}

int32_t orientation(coord p, coord q, coord r) {
    int32_t o = (q.lat - p.lat) * (r.lon - q.lon) - (q.lon - p.lon) * (r.lat - q.lat);

    if (o == 0) return COLLINEAR;

    return o > 0 ? CLOCKWISE : COUNTERCLOCKWISE;
}

bool do_intersect(coord p1, coord q1, coord p2, coord q2) {
    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    // General case
    if (o1 != o2 && o3 != o4)
        return true;

    // Special Cases
    // p1, q1 and p2 are collinear and p2 lies on segment p1q1
    if (o1 == COLLINEAR && on_segment(p1, p2, q1)) return true;

    // p1, q1 and q2 are collinear and q2 lies on segment p1q1
    if (o2 == COLLINEAR && on_segment(p1, q2, q1)) return true;

    // p2, q2 and p1 are collinear and p1 lies on segment p2q2
    if (o3 == COLLINEAR && on_segment(p2, p1, q2)) return true;

    // p2, q2 and q1 are collinear and q1 lies on segment p2q2
    if (o4 == COLLINEAR && on_segment(p2, q1, q2)) return true;

    return false;
}
