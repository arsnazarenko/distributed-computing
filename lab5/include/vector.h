#ifndef LAB4_VECTOR_H
#define LAB4_VECTOR_H

#include <stddef.h>
#include <stdio.h>
#include "ipc.h"

/**
 * key of request's queue: (time, id)
 */
typedef struct key {
    timestamp_t time;
    local_id id;
} key;

int lamport_time_compare(const key* lhs, const key* rhs);

typedef key data_t;
typedef int error_t;

struct vector;

typedef struct vector* vector_t;
typedef const struct vector* const_vector_t;

typedef int(*comparator)(const data_t* lhs, const data_t* rhs);

vector_t vector_init_with_cap(size_t capacity);
vector_t vector_init(void);
void vector_destroy(vector_t vec);

size_t vector_len(const_vector_t vec);
size_t vector_capacity(const_vector_t vec);

const data_t* vector_get(const_vector_t vec, size_t idx);
const data_t* vector_back(const_vector_t vec);
const data_t* vector_front(const_vector_t vec);

error_t vector_push_back(vector_t vec, data_t data);
error_t vector_pop_back(vector_t vec);

error_t vector_erase(vector_t vec, size_t idx);
error_t vector_insert(vector_t, size_t idx, data_t data);
error_t vector_set(vector_t, size_t idx, data_t data);
error_t vector_sort(vector_t vec, comparator comp);

void vector_print(const_vector_t vec, FILE* stream);

#endif //LAB4_VECTOR_H
