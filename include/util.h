// vim:ts=4:sw=4:expandtab
#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define FREE(p)          \
    do {                 \
        if (p != NULL) { \
            free(p);     \
            p = NULL;    \
        }                \
    } while (0)

#define DLOG(message, ...)                                                             \
    do {                                                                               \
        if (config.debug)                                                              \
            printf("[%s:%d] DEBUG: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define ELOG(message, ...)                                                                  \
    do {                                                                                    \
        fprintf(stderr, "[%s:%d] ERROR: " message "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

void bail(char *message);

long int parse_int(char *str);

double parse_double(char *str);

void parse_buttons_numbers(char *str, ignore_buttons_t *ignore_buttons);

window_jitter_t *parse_window_jitter(char* str);

void add_window_jitter(window_jitter_t **head, window_jitter_t *window_jitter);

long jitter_window_match(Window win);

long get_window_jitter(Window win);
