#ifndef __SKIP_LIST_H
#define __SKIP_LIST_H


#define SKIPLIST_MAX_LEVEL 10

typedef struct skiplist_node 
{    
    double key;  // key
    struct skiplist_node *backward;
    struct skiplist_node *forward[];  // next pointer array
} skiplist_node_t;

typedef struct skiplist 
{
    struct skiplist_node *header, *tail;
    unsigned long length;  // number count of node in level 0
    int level;  // skip list max level
} skiplist_t;




skiplist_t *skiplist_init();
int skiplist_search(skiplist_t *list, double key);
int skiplist_insert(skiplist_t *list, double key);
int skiplist_delete(skiplist_t *list, double key);
void skiplist_destory(skiplist_t *list);

#endif