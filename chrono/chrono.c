//
// Created by rleroux on 9/7/24.
//

#include "chrono.h"

#include <stdio.h>
#include <time.h>

#include "../utils/intersect.h"

struct gate_segment {
    coord a;
    coord b;
};

struct gates {
    int current_index;
    int count;
    struct gate_segment segments[];
};

struct gates gates = {.current_index = 0, .count = 3, .segments = {
    {.a = {.lon = 1, .lat = 1}, .b = {.lon = 2, .lat = 2} },
    {.a = {.lon = 1, .lat = 1}, .b = {.lon = 2, .lat = 2} },
    {.a = {.lon = 1, .lat = 1}, .b = {.lon = 2, .lat = 2} },
}};

coord previous_pos;
struct timespec previous_time;

void handle_position(const coord pos) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    //do stuff
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);

    printf("gps pos %d %d\n", pos.lon, pos.lat);

    const struct gate_segment current_gate_segment = gates.segments[gates.current_index];

    if (do_intersect(previous_pos, pos, current_gate_segment.a, current_gate_segment.b)) {

    }

    previous_pos = pos;
}

