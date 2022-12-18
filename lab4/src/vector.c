#include <malloc.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"

enum {
    DEFAULT_CAPACITY = 10,
    N = 2,
};

struct vector {
    data_t *buf;
    size_t cap;
    size_t len;
};


vector_t vector_init_with_cap(size_t capacity) {
    vector_t vec = (vector_t) malloc(sizeof(struct vector));
    if (vec) {
        data_t* buf = (data_t*) malloc(sizeof(data_t) * capacity);
        if (buf) {
            *vec = (struct vector) {.buf = buf, .cap = capacity, .len = 0};
        } else {
            free(vec);
            vec = NULL;
        }
    }
    return vec;
}
vector_t vector_init(void) {
    return vector_init_with_cap(DEFAULT_CAPACITY);
}

void vector_destroy(vector_t vec) {
    assert(vec);
    if (vec) {
        free(vec->buf);
        free(vec);
    }
}

size_t vector_len(const_vector_t vec) {
    assert(vec);
    return vec->len;
}
size_t vector_capacity(const_vector_t vec) {
    assert(vec);
    return vec->cap;
}

const data_t* vector_get(const_vector_t vec, size_t idx) {
    assert(vec);
    if (idx < vec->len) {
        return &(vec->buf[idx]);
    }
    return NULL;
}
const data_t* vector_back(const_vector_t vec) {
    assert(vec);
    if (vec->len > 0) {
        return &(vec->buf[vec->len - 1]);
    }
    return NULL;
}

const data_t* vector_front(const_vector_t vec) {
    assert(vec);
    if (vec->len > 0) {
        return &(vec->buf[0]);
    }
    return NULL;
}

error_t vector_push_back(vector_t vec, data_t data) {
    assert(vec);
    if (vec->cap == vec->len) {
        size_t new_cap = vec->cap * N;
        data_t* new_buf = (data_t*) realloc(vec->buf, sizeof(data_t) * new_cap);
        if (!new_buf) {
            return -1;
        }
        vec->buf = new_buf;
        vec->cap = new_cap;
    }
    vec->buf[vec->len++] = data;
    return 0;
}

error_t vector_pop_back(vector_t vec) {
    assert(vec);
    if (vec->len > 0) {
        --(vec->len);
        return 0;
    }
    return -1;
}

static void swap(data_t * lhs, data_t * rhs) {
    data_t tmp = *lhs;
    *lhs = *rhs;
    *rhs = tmp;
}

error_t sift_up_to_idx(vector_t vec, size_t pivot_idx) {
    assert(vec);
    if (pivot_idx < vec->len) {
       for (size_t i = vec->len - 1; i > pivot_idx; --i) {
           swap(&(vec->buf[i - 1]), &(vec->buf[i]));
       }
       return 0;
   }
   return -1;
}

error_t sift_idx_to_end(vector_t vec, size_t pivot_idx) {
    assert(vec);
    if (pivot_idx < vec->len) {
        for (size_t i = pivot_idx + 1; i < vec->len; ++i) {
            swap(&(vec->buf[i - 1]), &(vec->buf[i]));
        }
        return 0;
    }
    return -1;
}

error_t vector_erase(vector_t vec, size_t idx) {
    assert(vec);
    if (sift_idx_to_end(vec, idx) != -1) {
        vector_pop_back(vec);
        return 0;
    }
    return -1;
}

error_t vector_insert(vector_t vec, size_t idx, data_t data) {
    assert(vec);
    if (vector_push_back(vec, data) != -1) {
        sift_up_to_idx(vec, idx);
        return 0;
    }
    return -1;
}

error_t vector_set(vector_t vec, size_t idx, data_t data) {
    assert(vec);
    if (idx < vec->len) {
        vec->buf[idx] = data;
        return 0;
    }
    return -1;
}


error_t vector_sort(vector_t vec, comparator comp) {
    assert(vec);
    if (vec->len > 0) {
        qsort(vec->buf, vec->len, sizeof(data_t), (int(*)(const void*, const void*))comp);
    }
    return 0;
}

void vector_print(const_vector_t vec, FILE* stream) {
    assert(vec);
    fprintf(stream,
            "vec {\n"
            " cap = %zu\n"
            " len = %zu\n"
            " buf = %p\n"
            "[ ", vec->cap, vec->len, (void*) vec->buf);
    for(size_t i = 0; i < vec->len; ++i) {
        if (i != 0) {
            fprintf(stream, ", ");
        }
        fprintf(stream, "{%3d, %3d}", vec->buf[i].time, vec->buf[i].id);
    }
    fprintf(stream, " ]}\n");
}

