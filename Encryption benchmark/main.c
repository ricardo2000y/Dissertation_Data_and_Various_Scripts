#include "encryption.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define M 10000
#define N 1000
#define NUM_ITERATIONS 100

// Function to encrypt a file
double encrypt_file(const char* input_file, const char* output_file, encryption_vars_t* encryption_vars) {
    FILE *in_file, *out_file;
    uint32_t buffer[1024];
    size_t bytes_read;
    clock_t start, end;

    in_file = fopen(input_file, "rb");
    if (!in_file) {
        perror("Error opening input file");
        return -1;
    }

    out_file = fopen(output_file, "wb");
    if (!out_file) {
        perror("Error opening output file");
        fclose(in_file);
        return -1;
    }

    start = clock();

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), in_file)) > 0) {
        for (size_t i = 0; i < bytes_read / sizeof(uint32_t); ++i) {
            buffer[i] ^= key_generator(encryption_vars);
        }
        fwrite(buffer, 1, bytes_read, out_file);
    }

    end = clock();

    fclose(in_file);
    fclose(out_file);

    return ((double) (end - start)) / CLOCKS_PER_SEC;
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        printf("Usage: %s <map_type> <x1> <y1> <x2> <y2> <input_file>\n", argv[0]);
        printf("Map types: D - Duffing, L - Logistic, M - 2D-LM\n");
        return 1;
    }

    encryption_vars_t encryption_vars;
    char map_type = argv[1][0];
    switch (map_type) {
        case 'D':
        case 'd':
            encryption_vars.type = MAP_DUFFING;
            break;
        case 'L':
        case 'l':
            encryption_vars.type = MAP_LOGISTIC;
            break;
        case '2':
            encryption_vars.type = MAP_2D_LOGISTIC;
            break;
        default:
            printf("Invalid map type: %c\n", map_type);
            return 1;
    }

    encryption_vars.chaotic_map1.x = strtod(argv[2], NULL);
    encryption_vars.chaotic_map1.y = strtod(argv[3], NULL);
    encryption_vars.chaotic_map2.x = strtod(argv[4], NULL);
    encryption_vars.chaotic_map2.y = strtod(argv[5], NULL);

    encryption_vars.chaotic_map1.iterations = M;
    encryption_vars.chaotic_map2.iterations = N;

    msws32_var_t msws32_variables = {0};
    encryption_vars.msws32_variables = &msws32_variables;

    key_generator_setup(&encryption_vars);

    for (int k = 0; k < M; k++) {
        encryption_vars.chaotic_map_iterator(&encryption_vars.chaotic_map1);
    }

    const char* input_file = argv[6];
    char output_file[256];
    snprintf(output_file, sizeof(output_file), "%s.encrypted", input_file);

    double total_time = 0.0;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        double encryption_time = encrypt_file(input_file, output_file, &encryption_vars);
        if (encryption_time < 0) {
            printf("Error occurred during encryption.\n");
            return 1;
        }
        total_time += encryption_time;
    }

    double average_time = total_time / NUM_ITERATIONS;
    printf("Average encryption time for %s: %.6f seconds\n", input_file, average_time);

    return 0;
}