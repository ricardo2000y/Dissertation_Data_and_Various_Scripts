/**
 * @file encryption.c
 * @author Ricardo Jorge Dias Sampaio
 * @brief Implementation of a key generator based on various chaotic maps and the MSWS32 generator for the Secure VLC Project.
 * @version 2.0
 * @date 2024-08-27
 * \par License:
 * \ref mit_license "MIT License".
 *
 * @details This file contains the implementation of functions needed for the key generator
 * based on various chaotic maps and the MSWS32 generator, including initializations and key generation.
 */
#include "encryption.h"
#include <math.h>

static inline void doubleToUint64Bits(uint64_t* result, double d) {
    memcpy(result, &d, sizeof(double));
}

static void duffing_map_iteration(chaotic_map_t* map) {
    double temp_x = map->y;
    map->y = -DUFFING_BETA * map->x + DUFFING_ALPHA * temp_x - (temp_x * temp_x * temp_x);
    map->x = temp_x;
}

static void logistic_map_iteration(chaotic_map_t* map) {
    map->x = LOGISTIC_R * map->x * (1 - map->x);
    map->y = LOGISTIC_R * map->y * (1 - map->y);
}

static void logistic2D_map_iteration(chaotic_map_t* map) {
    map->x = LOGISTIC2D_R * (3 * map->y + 1) * map->x * (1 - map->x);
    map->y = LOGISTIC2D_R * (3 * map->x + 1) * map->y * (1 - map->y);
}

static chaotic_map_iterator_t get_chaotic_map_iterator_t(map_type_t type) {
    switch (type) {
        case MAP_DUFFING: return duffing_map_iteration;
        case MAP_LOGISTIC: return logistic_map_iteration;
        case MAP_2D_LOGISTIC: return logistic2D_map_iteration;    
        default:
            return NULL;
    }
}

static inline uint32_t msws32(msws32_var_t *msws32_variables) {
    msws32_variables->x *= msws32_variables->x;
    msws32_variables->x += (msws32_variables->w += msws32_variables->s);
    return msws32_variables->x = (msws32_variables->x >> 32) | (msws32_variables->x << 32);
}

static void initialize_generator(chaotic_map_iterator_t chaotic_map_iterator, chaotic_map_t* chaotic_map_vars) {
    for (int i = 0; i < chaotic_map_vars->iterations; i++) {
        if (chaotic_map_iterator == NULL) {
            return;
        }
        chaotic_map_iterator(chaotic_map_vars);
    }
}

void key_generator_setup(encryption_vars_t* encryption_vars) {
    encryption_vars->chaotic_map_iterator = get_chaotic_map_iterator_t(encryption_vars->type);
    if (encryption_vars->chaotic_map_iterator == NULL) {
        return;
    }

    initialize_generator(encryption_vars->chaotic_map_iterator, &encryption_vars->chaotic_map1);
    initialize_generator(encryption_vars->chaotic_map_iterator, &encryption_vars->chaotic_map2);

    doubleToUint64Bits(&(encryption_vars->msws32_variables->x), encryption_vars->chaotic_map2.y);
    doubleToUint64Bits(&(encryption_vars->msws32_variables->s), encryption_vars->chaotic_map2.y);
}

uint32_t key_generator(encryption_vars_t* encryption_vars) {
    encryption_vars->chaotic_map_iterator(&encryption_vars->chaotic_map1);
    doubleToUint64Bits(&(encryption_vars->msws32_variables->w), encryption_vars->chaotic_map1.y);
    if(encryption_vars->type==MAP_LOGISTIC) {
        // because x and y are not related in the logistic map 
        uint64_t temp;
        doubleToUint64Bits(&temp, encryption_vars->chaotic_map1.x);
        encryption_vars->msws32_variables->w ^= temp;
        }
    return msws32(encryption_vars->msws32_variables);
}