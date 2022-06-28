#include <stdio.h>
#include "skiplist.h"


int main()
{
    skiplist_t *mylist = skiplist_init();
    int i;
    for(i = 0; i < 10000000; i++)
    {
        skiplist_insert(mylist, i);
    }

    //print_stat();
    int ret = skiplist_search(mylist, 99);
    printf("search result %d\n", ret);

    ret = skiplist_search(mylist, 999);
    printf("search result %d\n", ret);


    ret = skiplist_search(mylist, 9.9);
    printf("search result %d\n", ret);


    ret = skiplist_delete(mylist, 89);
    printf("search delete %d\n", ret);

    //skiplist_print_all(mylist);
    //skiplist_print_all_(mylist);
    skiplist_insert(mylist, 66666);
    ret = skiplist_search(mylist, 66666);
    printf("search for 66666 result %d\n", ret);

    skiplist_destory(mylist);

    printf("second init skip list:\n");
    mylist = skiplist_init();
    skiplist_insert(mylist, 66666);
    ret = skiplist_search(mylist, 66666);
    printf("search result %d\n", ret);
    ret = skiplist_search(mylist, 6);
    printf("search result %d\n", ret);
    ret = skiplist_delete(mylist, 66666);
    printf("search delete %d\n", ret);
    ret = skiplist_search(mylist, 66666);
    printf("after delete search result %d\n", ret);
}
