#include <cmath>

typedef char s8;
typedef unsigned char u8;
typedef short s16;
typedef unsigned short u16; 
typedef int s32;
typedef unsigned int u32; 
typedef long long s64;
typedef unsigned long long u64;

#define global_variable static
#define positive_negative (rand() % 2 * 2 - 1)

const float PI = 3.14159f, pheromone_sense_angle = PI * 0.5f, pheromone_turn_angle = 0.05f;
float a;


bool less_or_equal(float lesser_num, float higher_num, bool more_equal) {
    if (more_equal) {
        return lesser_num <= higher_num;
    }
    else {
        return lesser_num < higher_num;
    }
}

float get_angle(float delta_x, float delta_y, float distance, float rotation) {
    if (delta_x < 0) {
        if (delta_y < 0) {
            a = abs(asin(delta_y / distance)) + PI;
        }
        else {
            a = PI - (asin(delta_y / distance));
        }
    }
    else if (delta_y < 0) {
        a = 2 * PI - abs(asin(delta_y / distance));
    }
    else {
        a = asin(delta_y / distance);
    }
    if (a - rotation < -PI) {
        return 2.f * PI - rotation + a;
    }
    else if (a - rotation > PI) {
        return a - 2.f * PI - rotation;
    }
    else {
        return a - rotation;
    }
}

float get_delta_x(float ant_x, float target_x) {
	return target_x - ant_x;
}float get_delta_y(float ant_y, float target_y) {
	return ant_y - target_y;
}

float get_distance(float ant_x, float ant_y, float target_x, float target_y) {
	return sqrtf(powf(target_x - ant_x, 2) + powf(ant_y - target_y, 2));
}

float pheromone_function(float angle) {
    int concentration_left = 0;
    int concentration_front = 0;
    int concentration_right = 0;
    if (angle > pheromone_sense_angle / 3) {
        concentration_left++;
    } 
    else if (angle < -pheromone_sense_angle / 3) {
        concentration_right++;
    }
    else {
        concentration_right++;
    }
    if (concentration_left > concentration_right && concentration_left > concentration_front) {
        return pheromone_turn_angle;
    }
    else if (concentration_right > concentration_front) {
        return -pheromone_turn_angle;
    }
    else {
        return 0.f;
    }
}

inline int
clamp(int min, int val, int max) {
	if (val < min) return min;
	if (val > max) return max;
	return val;
}