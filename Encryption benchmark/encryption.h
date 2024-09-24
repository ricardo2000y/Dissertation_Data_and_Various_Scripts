/**
 * @file encryption.h
 * @author Ricardo Jorge Dias Sampaio
 * @brief Header file for the key generator based on various chaotic maps and the MSWS32 generator for the Secure VLC Project.
 * @version 2.0
 * @date 2024-08-27
 * \par License:
 * \ref mit_license "MIT License".
 * @details This header file includes the declarations of functions, data structures, and constants
 * required for the implementation of the key generator based on various chaotic maps and the MSWS32 generator for the Secure VLC Project.
 */

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <stdint.h>
#include <string.h>


// Constants for map parameters
#define DUFFING_ALPHA 2.75
#define DUFFING_BETA 0.2
#define LOGISTIC_R 3.99
#define LOGISTIC2D_R 1.19


/**
 * @brief Enumeration of supported chaotic maps.
 */
typedef enum {
    MAP_DUFFING,
    MAP_LOGISTIC,
    MAP_2D_LOGISTIC,
    // Add other maps as needed
} map_type_t;

/**
 * @brief Structure for MSWS32 generator variables.
 */
typedef struct msws32_var_t {
    uint64_t x;
    uint64_t w;
    uint64_t s;
} msws32_var_t;

/**
 * @brief Structure for chaotic map variables.
 */
typedef struct chaotic_map_t {
    double x;
    double y;
    int iterations;
} chaotic_map_t;

/**
 * @brief Function pointer type for map iterations.
 */
typedef void (*chaotic_map_iterator_t)(chaotic_map_t*);

/**
 * @brief Structure for encryption variables.
 */
typedef struct encryption_vars_t {
    map_type_t type;
    chaotic_map_iterator_t chaotic_map_iterator;
    chaotic_map_t chaotic_map1;
    chaotic_map_t chaotic_map2;
    msws32_var_t* msws32_variables;
} encryption_vars_t;

/**
 * @brief Sets up the key generator.
 * @param encryption_vars Pointer to the encryption_vars_t structure.
 */
void key_generator_setup(encryption_vars_t* encryption_vars);

/**
 * @brief Generates a new key using the chaotic map and MSWS32 generator.
 * @param encryption_vars Pointer to the encryption_vars_t structure.
 * @return The new key generated.
 */
uint32_t key_generator(encryption_vars_t* encryption_vars);

#endif // ENCRYPTION_H