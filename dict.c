#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dict.h"

/* Code within #if DEBUG ... #endif blocks will not be compiled
 * when DEBUG set to zero (it will be removed by the C preprocessor).
 *
 * set to 1 to display bin fill & pause for 3sec upon rehash *
 * set to 0 to disable bin fill & pause upon rehash          */
#define DEBUG 0


/* the infamous djb2 hash (without bitwise operators) */
static unsigned long _hash(const char *key)
{
    int c;

    unsigned long hash = 5381;

    while (c = *key++)
        hash = hash * 33 + c;

    return hash;
}


#if DEBUG
static void dict_debug_bin_fill(struct dict *D)
{
    unsigned int idx;
    unsigned long int n;
    struct list *cur;

    for (idx=0, n=0; idx<D->nbins; idx++, n=0) {
        for(cur=D->bins[idx]; cur; cur=cur->next)
            n++;

        fprintf(stderr, "bin[%d]: %lu\n", idx, n);
    }

}
#endif


static int dict_rehash(struct dict *D)
{
    int idx;

    int new_idx;
    unsigned int new_nbins;
    struct list **new_bins;

    struct list *cur;

#if DEBUG
    /* print # of items in each bin */
    dict_debug_bin_fill(D);
    fprintf(stderr, "Rehashing...\n");
    /* pause for 3 seconds */ 
    sleep(3);
#endif

    /**************************************/
    /* Write your code for rehashing here */
    /**************************************/

    /* malloc new bin twice as large */
  new_nbins = 2 * D->nbins;
  new_bins = malloc(new_nbins*sizeof(*D->bins));

  /* Move list nodes over, iterate using Lab 9 
    +- Get stored hash
    +- hash mod new bins
    - move node 
    */

  for (idx=0; idx<D->nbins; idx++) 
  {
    cur = D->bins[idx];
    while(D->bins[idx] != NULL) 
    {
      new_idx = D->bins[idx]->hash % new_nbins; /*bin #*/
      new_bins[idx] = NULL; /* init to NULL */

      /* Move D->bins[idx] to new location */
      cur = malloc(sizeof(*cur));
      cur->hash = D->bins[idx]->hash; 
      cur->key = D->bins[idx]->key;
      cur->user_data = D->bins[idx]->user_data;
      
      /* insert cur into new_bins */
      cur->next = new_bins[new_idx];
      new_bins[new_idx] = cur;      
      D->bins[idx] = D->bins[idx]->next;
    }
  }

/*  This code is a gift. 
    I have commented it out so that your program doesn't crash
    (since you haven't implemented the rest of rehashing yet)
    */
    free(D->bins);
    D->bins = new_bins;
    D->nbins = new_nbins;


    return 0;
}


int dict_init(struct dict *D, void (*deleter)(void* user_data))
{
    int i;

    D->nbins = 6;
    D->bins = malloc(D->nbins*sizeof(*D->bins));
    if (!D->bins)
        return 1;

    for (i=0; i<D->nbins; i++)
        D->bins[i] = NULL;

    D->deleter = deleter;
    D->count = 0;   /* this line is new */

    return 0;
}


void dict_destroy(struct dict *D)
{
    unsigned int idx;
    struct list *cur;

    for (idx=0; idx<D->nbins; idx++) {
        while(D->bins[idx]) {
            cur = D->bins[idx];
            D->bins[idx] = cur->next;

            if (D->deleter)
                D->deleter(cur->user_data);

            free(cur->key);
            free(cur);
        }
    }

    free(D->bins);
}


int dict_insert(struct dict *D, const char *key, void *user_data)
{
    float load = (float)(D->count + 1) / D->nbins;
    unsigned int idx = _hash(key) % D->nbins;
    struct list *item;

    /* Rehash if load factor is not less than 0.75 ***********/
    if (load >= 0.75)
        if (dict_rehash(D))
            return 1;  /* rehash (& insertion) failed */
    /*********************************************************/

    item = malloc(sizeof(*item));

    if (!item) {
        fprintf(stderr, "Insert Failed\n");
        return 1;
    }

    item->key = malloc(strlen(key)+1);
    if (!item->key) {
        fprintf(stderr, "Insert Failed\n");
        free(item);
        return 1;
    }

    strcpy(item->key, key);

    item->hash = _hash(key);
    item->user_data = user_data;

    item->next = D->bins[idx];
    D->bins[idx] = item;
    D->count++;     /* this line is new */

    return 0;
}


void dict_delete(struct dict *D, const char *key)
{
    void *user_data;

    if (D->deleter && (user_data = dict_pop(D, key)))
        D->deleter(user_data);
}


void *dict_peek(struct dict *D, const char *key)
{
    struct list *cur;

    unsigned long hash = _hash(key);
    unsigned int idx = hash % D->nbins;

    for (cur=D->bins[idx]; cur; cur=cur->next)
        if (hash == cur->hash)
            if (!strcmp(cur->key, key))
                return cur->user_data;

    return NULL;
}


void *dict_pop(struct dict *D, const char *key)
{
    struct list *cur;
    void *user_data;

    unsigned long hash = _hash(key);
    unsigned int idx = hash % D->nbins;
    struct list **prev_next = &D->bins[idx];

    for (cur=D->bins[idx]; cur; cur=cur->next) {
        if (hash == cur->hash)
            if (!strcmp(cur->key, key)) {
                *prev_next = cur->next;
                user_data = cur->user_data;
                free(cur->key);
                free(cur);
                D->count--;     /* this line is new */
                return user_data;
            }

        prev_next = &cur->next;
    }

    return NULL;
}


/* this function is new */
float dict_loadfactor(struct dict *D)
{
    return (float)D->count / D->nbins;
}