#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include "skiplist.h"


// useful for log debug
#define FUNC_ENTRY printf("%s begin at line %d\n", __func__, __LINE__)
#define FUNC_EXIT printf("%s end at line %d\n", __func__, __LINE__)


// because skip list used in redis and leveldb i do it
//--------------------------------------------
// copy from redis
static void zmalloc_oom(size_t size) {
    fprintf(stderr, "zmalloc: Out of memory trying to allocate %zu bytes\n",
        size);
    fflush(stderr);
    abort();
}

void *zmalloc(size_t size) {
    void *ptr = malloc(size);

    if (!ptr) zmalloc_oom(size);

    return ptr;
}

//---------------------------------from redis

skiplist_node_t *skiplist_create_node(int level, double key)
{
    skiplist_node_t *node = zmalloc(sizeof(*node)+level*sizeof(skiplist_node_t *));
    
    node->key = key;
    return node;
}



skiplist_t *skiplist_init()
{
    skiplist_node_t *header = NULL;  // dummy node
    skiplist_t *list = malloc(sizeof(*list));
    list->level = 1; // default max level 
    // set header key to 0, it not used for compare
    header = skiplist_create_node(SKIPLIST_MAX_LEVEL, 0);
    list->header = header;

    int i;
    for(i = 0; i < SKIPLIST_MAX_LEVEL; i++)
    {
        header->forward[i] = NULL;
    }
    header->backward = NULL;

    list->tail = NULL;
    return list;
}


// define double equation funtion
int double_cmp(double a, double b)
{
    return fabs(a-b) < DBL_EPSILON;
}


// 1:found, 0:not found
int skiplist_search(skiplist_t *list, double key)
{

    skiplist_node_t *cur = NULL;
    int i = 0;
    cur = list->header;
    
    for(i = list->level - 1; i >= 0; i--)
    {
        //printf("level[%d] ", level);
        while(cur->forward[i] && (cur->forward[i]->key) < key)
        {
            //printf("%f -->", cur->forward[level]->key);
            cur = cur->forward[i];
        }
        //printf("inf\n");
        // down to next level    
    }

    // because cur is the last less than key, so the next element must equal or bigger than key
    // so focus on the next elemen(forward pointer)
    skiplist_node_t *x = cur->forward[0];
    if(x == NULL)
        return 0; // not found

    
    return double_cmp(x->key, key);
}



static int count[SKIPLIST_MAX_LEVEL] = {0};
static void level_stat(int level)
{
    int index = level - 1;
    count[index]++;
}

void print_stat()
{
    int i;
    int *array = count;
    size_t len = SKIPLIST_MAX_LEVEL;
    int total = 0;
    for(i = 0; i < len; i++)
    {
        total += *(array + i);
        printf("level %i: %d\n", i, *(array + i));
    }
    printf("======================\n\ttotal %d\n", total);
}



// generate a random level for a new data node
static int random_level(void)
{
    int level = 1;
    while((random()&0xffff) < (0.25 * 0xffff))
        level += 1;

    return (level < SKIPLIST_MAX_LEVEL) ? level:SKIPLIST_MAX_LEVEL;
}


int skiplist_insert(skiplist_t *list, double key)
{
    //FUNC_ENTRY;
    
    int i;
    skiplist_node_t *cur = NULL;
    // previous node info before the new node, callled "search path"
    skiplist_node_t *prev_info[SKIPLIST_MAX_LEVEL] = {NULL};

    if(list == NULL)
    {
        fprintf(stderr, "input parameter is null\n");
        return -1;
    }
    
    cur = list->header;
    for(i = list->level - 1; i >= 0; i--)
    {
        
        while(cur->forward[i] && cur->forward[i]->key < key)
        {
            cur = cur->forward[i]; // set to the next at same level
        }

        prev_info[i] = cur;
        // down to next level    
    }

    int rand_level = 0;
    // generate rand level
    rand_level = random_level();
    if(rand_level > list->level)
    {
        for(i = list->level; i < rand_level; i++)
        {
            prev_info[i] = list->header;
        }
        // update skip list level
        list->level = rand_level;
    }
    
    //level_stat(rand_level);
    skiplist_node_t *x = skiplist_create_node(rand_level, key);
    for(i = rand_level-1; i >= 0; i--)  // insert x to link list
    {
        x->forward[i] = prev_info[i]->forward[i];
        prev_info[i]->forward[i] = x;
    }

    //if x has the forward node
    skiplist_node_t *x_forward = x->forward[0]; 
    if(x_forward)
        x_forward->backward = x;
    else
        list->tail = x;

    // x's backward should point to valid data node
    if(prev_info[0] == list->header)
        x->backward = NULL;
    else
        x->backward = prev_info[0];

    list->length++;
    //FUNC_EXIT;
    return 0;
}


#if 0
void skiplist_print_all(skiplist_t *list)
{

    skiplist_node_t *cur = list->header->forward[0];
    while(cur)
    {
        printf("%f    ", cur->key);
        cur = cur->forward[0];
    }
    printf("\n");
}
#endif


int skiplist_delete(skiplist_t *list, double key)
{

    // step 1 search and get the target position
    // step 2 rearragement the prev and next pointers mainly level[0] links 
    // step 3 free it from skip list
    int i;
    skiplist_node_t *cur = NULL;
    // previous node info before to insert callled "search path"
    skiplist_node_t *prev_info[SKIPLIST_MAX_LEVEL] = {NULL};
    cur = list->header;

    for(i = SKIPLIST_MAX_LEVEL - 1; i >= 0; i--)
    {
        prev_info[i] = list->header;
    }

    
    for(i = list->level - 1; i >= 0; i--)
    {
        
        while(cur->forward[i] && cur->forward[i]->key < key)
        {
            cur = cur->forward[i]; // set to the next at same level
        }

        prev_info[i] = cur;
        // down to next level    
    }

    skiplist_node_t *x = cur->forward[0];
    if(x == NULL)
        return -1; // not found

    if(double_cmp(x->key, key) == 1)
    {
        // have found it
        for(i = SKIPLIST_MAX_LEVEL - 1; i >= 0; i--)
        {
            #if 0
            printf("delete i %d\n", i);
            if(prev_info[i] == list->header &&
               prev_info[i]->forward[i] == NULL)    // no list at the level
                continue;

            prev_info[i]->forward[i] = x->forward[i];
            #endif
            
            if(prev_info[i]->forward[i] == x)  // x in the line
            {
                prev_info[i]->forward[i] = x->forward[i];

                // only have x node at the level
                if(prev_info[i] == list->header &&
                   x->forward[i] == NULL)
                   list->level --;
            }
   
        }
        
        // x's next node
        skiplist_node_t *x_forward = x->forward[0];
        if(x_forward)
        {
            x_forward->backward = x->backward;   
        }
        else
        {
            list->tail = x->backward;
        }

        free(x);
        list->length--;
        
        return 0;   
    }

    return -1;
}


void skiplist_destory(skiplist_t *list)
{
    if(list == NULL)
        return;

    skiplist_node_t *p = list->header;
    skiplist_node_t *next = NULL;
    while(p)
    {
        next = p->forward[0];
        free(p);
        p = next;
    }

    free(list);
}

