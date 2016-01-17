#ifndef _MB_NODE_H_
#define _MB_NODE_H_


#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//A multi-bit node data structure

#if __SIZEOF_LONG__ == 8
#define BITMAP_TYPE uint64_t
#define STRIDE 6
#define BITMAP_BITS (1<<STRIDE)
#else
#define BITMAP_TYPE uint32_t
#define STRIDE 5 
#define BITMAP_BITS (1<<STRIDE)
#endif

#define POINT(X) ((struct mb_node*)(X))
#define NODE_SIZE  (sizeof(struct mb_node))
#define FAST_TREE_FUNCTION

struct mb_node{
    BITMAP_TYPE external;
    BITMAP_TYPE internal;
    void     *child_ptr;
}__attribute__((aligned(8)));


static inline uint32_t UP_RULE(uint32_t x)
{
    return (x*sizeof(void*) + NODE_SIZE - 1)/(NODE_SIZE);
}

static inline uint32_t UP_CHILD(uint32_t x)
{
    return x;
}



//count 1's from the right of the pos, not including the 1
//pos = 0 ~ 64
//pos == 64 means to counts how many 1's in the bitmap
//I'm not sure this is a implementation specific.
static inline int count_ones(BITMAP_TYPE bitmap, uint8_t pos)
{
//    if (pos == 0)
//        return 0;
//    return __builtin_popcountl((bitmap<<(64 - pos)));
#if __SIZEOF_LONG__ == 8 
    return __builtin_popcountl(bitmap>>pos) - 1;
#else
    return __builtin_popcountll(bitmap>>pos) - 1;
#endif

}

static inline int count_children(BITMAP_TYPE bitmap)
{
#if __SIZEOF_LONG__ == 8
    return __builtin_popcountl(bitmap);
#else
    return __builtin_popcountll(bitmap);
#endif
}

static inline uint32_t count_inl_bitmap(uint32_t bit, int cidr)
{
    uint32_t pos = (1<<cidr) + bit;
    return (pos - 1);
}

static inline uint32_t count_enl_bitmap(uint32_t bits)
{
    return (bits);
}

static inline void update_inl_bitmap(struct mb_node *node, int pos)
{
    node->internal |= (1ULL << pos);
}

static inline void update_enl_bitmap(struct mb_node *node, int pos)
{
    node->external |= (1ULL << pos);
}

static inline BITMAP_TYPE test_bitmap(BITMAP_TYPE bitmap, int pos)
{
    return (bitmap & (1ULL << pos));
}


// ----child_ptr-----
// --------|---------
// |rules  | child--| 
// |-------|--------|
// to get the head of the memory : POINT(x) - UP_RULE(x)
// 
#include "mm.h"
void *new_node(struct mm* mm, int mb_cnt, int result_cnt, int level);
void free_node(struct mm *mm, void *ptr, uint32_t cnt, int level);

struct mb_node * extend_child(struct mm *mm, struct mb_node *node,
        int level, uint32_t pos);

void extend_rule(struct mm *mm, struct mb_node *node, uint32_t pos,
        int level, void *nhi);
void reduce_child(struct mm *mm, struct mb_node *node, int pos, int level);
void reduce_rule(struct mm *mm, struct mb_node *node, uint32_t pos,
        int level);

//void mem_subtrie(struct mb_node *n, struct mem_stats *ms);
int tree_function(BITMAP_TYPE bitmap, uint8_t stride);
int find_overlap_in_node(BITMAP_TYPE bitmap, uint8_t stride, uint8_t *mask, int limit_inside);

static inline void clear_bitmap(BITMAP_TYPE *bitmap, int pos)
{
    *bitmap &= (~(1ULL << pos));
}

static inline void set_bitmap(BITMAP_TYPE *bitmap, int pos)
{
    *bitmap |= (1ULL<<pos);
}
void destroy_subtrie(struct mb_node *node, struct mm *m, void (*destroy_nhi)(void *nhi), int depth);

//GCC optimize
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)


#endif