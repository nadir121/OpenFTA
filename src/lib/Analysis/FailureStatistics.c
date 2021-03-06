///////////////////////////////////////////////////////////////////////////////
// 
// OpenFTA - Fault Tree Analysis
// Copyright (C) 2005 FSC Limited
// 
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your 
// option) any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
// more details.
//
// You should have received a copy of the GNU General Public License along 
// with this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// To contact FSC, please send an email to support@fsc.co.uk or write to 
// FSC Ltd., Cardiff Business Technology Centre, Senghenydd Road, Cardiff,
// CF24 4AY.
//
///////////////////////////////////////////////////////////////////////////////

/*****************************************************************
 Module : FailureStatistics
 Author : FSC Limited
 Date   : 24 July 1996
 Origin : FTA project,
          FSC Limited Private Venture.
 
 SccsId : @(#)FailureStatistics.c	1.2 09/26/96
 
 Module for storing failure statistics
 
 Single-instance data structure.
 
   use initialise_failures() to initialise with a maximum number of
                             failure modes
 
   use record_failure()      to record each failure event
 
   use initialise_failures() again to free memory
 
 A failure is represented by a bit array (see bits.h and bits.c),
 in which the bits set correspond to the basic failure events which
 occured. This is called a failure vector.
 
 The module was written primarily for recording failures
 generated with the monte-carlo method. It may later be used also
 for cut sets generated by other means.

 The failures are stored in a binary tree ordered on the integer value
 of the failure vector. (Note that this may cause problems if the failures
 are generated in order). Each record also contains a count field (number
 of occurences of that failure).

 There is a single instance of the tree. It is headed by the static
 variable Failtree.

 To maintain the rank information the nodes of the tree are also
 elements of a doubly-linked linked list. The fields next and prev
 point to the next LOWEST and next HIGHEST counts in the tree. Since the
 rank information will not change much as failures are added to the tree,
 this array is kept up to date as we go along. A new failure mode is always
 added at the bottom of the list. If a failure mode becomes
 more frequent then the one immediately above it, the two
 are swapped.

 It is forseen than an unreasonable number of distinct failure modes
 might be generated (e.g. if large probabilities of failure occur in the
 Monte-Carlo simulation of a large fault tree). For this reason a
 limit may be set on the number of different failure modes recorded.
 When this number is exceeded no new failure modes are recorded:
 instead, the static
 variable nother is incremented. Note that in a Monte-Carlo
 simulation the most likely failure modes will probably be generated
 first, and it will be the least likely that are rejected.

*****************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "FailureStatistics.h"

#include "MemoryUtil.h"

#define SUBSET        1
#define SUPERSET     -1
#define NEITHER       0

static Fnode  *Failtree = NULL; /* head of tree                         */
static Fnode  *High = NULL;     /* high end of the order list           */
static Fnode  *Low  = NULL;     /* low end of the order list            */
static int     nfm = 0;         /* next place in rank array             */
static int     max_fail = 0;    /* max number of failure modes          */
static int     nother = 0;      /* number of failures mode not recorded */

/*---------------------------------------------------------------
 Routine : Fnode_create
 Purpose : Create a Fnode and initialise data
---------------------------------------------------------------*/
static Fnode *
  Fnode_create( BitArray *b )
{
/*     Fnode *f = (Fnode *)malloc( sizeof(Fnode) ); */
    Fnode *f;

    if ( !fNewMemory( (void **)&f, sizeof(Fnode) ) )
    {
      exit( 1 );
    }

    f->b = b;
    f->n = 0;
    f->left = NULL;
    f->right = NULL;
    f->next = NULL;
    f->prev = NULL;

    return f;

} /* Fnode_create */

/*---------------------------------------------------------------
 Routine : order_swap
 Purpose : Swap an Fnode with the next Fnode in the order list.
---------------------------------------------------------------*/
static void
  order_swap(Fnode *p)
{
    Fnode *tmp;

    if (p->next) {
        if ( ( tmp = p->next->next ) ) 
        {
            p->next->next->prev = p;
        }
        p->next->next = p;

        if (p->prev) {
            p->prev->next = p->next;
        }

        p->next->prev = p->prev;
        p->prev = p->next;
        p->next = tmp;
    }
} /* order_swap */

/*---------------------------------------------------------------
 Routine : tree_insert
 Purpose : Insert an Fnode structure in the tree.

 Standard C tree. see K&R p141.
 Note also code for maintaining rank array.
---------------------------------------------------------------*/
static Fnode *
  tree_insert( Fnode *t, BitArray *b )
{
    BitArray *bc;
	
	if ( t == NULL ) {
		/* create and insert new Fnode */
        if ( (max_fail == 0) || (nfm <= max_fail) ) {
			bc = BitClone(b);
            t = Fnode_create( bc );
            t->n = 1;
            if (High == NULL) {
                High = Low = t;
            } else {
                Low->prev = t;
                t->next = Low;
                Low = t;
            }
            nfm++;
        } else {
            nother++;
        }
    } else {
        switch ( BitComp(b, t->b) ) {
        case 0 :                           /* found it */
            t->n++;
            while( t != NULL && t->next != NULL && t->n > t->next->n ) {
                if (t == Low) Low = t->next;
                if (t->next == High) High = t;
                order_swap(t);
            }
            break;
        case 1 :
            t->right = tree_insert( t->right, b );
            break;
        case -1 :
            t->left = tree_insert( t->left, b );
            break;
        }
    }
    return t;

} /* tree_insert */

/*---------------------------------------------------------------
 Routine : tree_print
 Purpose : Print the data in the tree, from smallest bit array to
 largest.

 This routine is recursive.
---------------------------------------------------------------*/
static void
  tree_print( Fnode *t )
{
    if ( t != NULL ) {
        tree_print( t->left );
        printf("%s - %d\n", BitString(t->b), t->n);
        tree_print( t->right );
    }
} /* tree_print */

/*---------------------------------------------------------------
 Routine : list_print
 Purpose : Print fail data as a list to stdout
---------------------------------------------------------------*/
void
  list_print()
{
    Fnode *p;
    int i = 0;

    for (p = High; p != NULL; p = p->prev) {
        printf("(%-3d) %s - %d\n", i++, BitString(p->b), p->n);
    }
} /* list_print */

/*---------------------------------------------------------------
 Routine : tree_destroy
 Purpose : Destroy the whole tree, free all memory
---------------------------------------------------------------*/
static void
  tree_destroy( Fnode *t )
{
    if ( t != NULL ) {
        BitDestroy(t->b);
        tree_destroy(t->left);
        tree_destroy(t->right);
        FreeMemory( t );
    }
} /* tree_destroy */

/*---------------------------------------------------------------
 Routine : initialise_failures
 Purpose : Initialise data structures for the recording of
 failure information.
---------------------------------------------------------------*/
void
  initialise_failures( int n )   /* IN  - max No failure modes */
{
    tree_destroy(Failtree);
    Failtree = NULL;
    High = NULL;
    Low = NULL;
    max_fail = n;
    nfm = 0;
    nother = 0;

} /* initialise_failures */

/*---------------------------------------------------------------
 Routine : record_failure
 Purpose : Record an instance of the failure represented by bit
 array b

 Make a copy and insert it in the tree.
---------------------------------------------------------------*/
void
  record_failure(BitArray *b)   /* IN - bit array (failure mode) */
{
    Failtree = tree_insert(Failtree, b);

} /* record_failure */

/*---------------------------------------------------------------
 Routine : get_fail_data
 Purpose : Return the failure data
---------------------------------------------------------------*/
void
  get_fail_data(
    Fnode **h,      /* OUT -  top of list                      */
    int    *n,      /* OUT -  number of failure modes recorded */
    int    *oth )   /* OUT -  number of "other" failures       */
{
    *h   = High;
    *n   = nfm;
    *oth = nother;

} /* get_fail_data */

/*---------------------------------------------------------------
 Routine : unlink_node
 Purpose : Remove a node from the tree. 

 Does not free memory!
---------------------------------------------------------------*/
static void
  unlink_node( Fnode *f )
{
    if (f->prev == NULL) {
        Low = f->next;
    } else {
        f->prev->next = f->next;
    }

    if (f->next == NULL) {
        High = f->prev;
    } else {
        f->next->prev = f->prev;
    }

 /* f->n = 0; */
    nfm--;

} /* unlink_node */

/*---------------------------------------------------------------
 Routine : FnodeSubset
 Purpose : Check subset status between two fnodes

 Return SUBSET    if f1 is a subset of f2
        SUPERSET  if f2 is a subset of f1
        NEITHER   if neither is a subset of the other

 BEGIN GroupEqual
    calculate b = f1->b OR f2->b
    IF (b == f2->b) THEN
       SUBSET
    ELSE IF (b == f1->b) THEN
       SUPERSET
    ELSE
       NEITHER
    END IF
 END GroupEqual
---------------------------------------------------------------*/
static int
  FnodeSubset(Fnode *f1, Fnode *f2)
{
    int    result;
    BitArray *b = BitOR(f1->b, f2->b);

    if ( BitEquals(b, f2->b) ) {
        result =  SUBSET;
    } else if ( BitEquals(b, f1->b) ) {
        result = SUPERSET;
    } else {
        result = NEITHER;
    }
    BitDestroy(b);
    return result;
} /* FnodeSubset */


/*---------------------------------------------------------------
 Routine : add_fnode
 Purpose : adds an fnode to the given list
---------------------------------------------------------------*/
void
  add_fnode( Fnode *node, Fnode **list)
{
    if(!(*list))
    {
        /* only item in list */
        *list = node;
    }
    else
    {
        /* add to front of list */
        (*list)->prev = node;
        node->next = *list;
        *list = node;
    }
}


/*---------------------------------------------------------------
 Routine : merge_fnode
 Purpose : adds an list2 to the end of list 1
---------------------------------------------------------------*/
void
  merge_fnodes( Fnode *list1, Fnode *list2)
{
    Fnode *p;

    /* check for empty lists */
    if(!list1)
    {
        list1 = list2;
    }
    else if(list1 && list2)
    {
        /* find end of list 1 */   
        p = list1;
        while(p->next)
        {
            p = p->next;
        }

        /* merge lists */
        p->next = list2;
        list2->prev = p;
    }
}


/*---------------------------------------------------------------
 Routine : order_fail_data
 Purpose : orders fail data into a list increasing in length
---------------------------------------------------------------*/
Fnode *
  order_fail_data2( Fnode *list)
{
    Fnode *p;
    Fnode *local_p;
    Fnode *larger = NULL;
    Fnode *smaller = NULL;
    Fnode *merged = NULL;
    int    comparison;

    p = list;
    comparison = BitCount(p->b);
    
    while(p != NULL)
    {
        local_p = p;
        p = p->next;

        /* remove from current list */
        unlink_node(local_p);

        /* split data into two lists */
        if(BitCount(local_p->b) <= comparison)
        {
            add_fnode(local_p, &smaller);
        }
        else
        {
            add_fnode(local_p, &larger);
        }
    }

    /* merge lists back together */
    if(!smaller)
    {
        merged = larger;
    }
    else if (!larger)
    {
        merged = smaller;
    }
    else
    {
        merged = smaller;
        merge_fnodes(merged, larger);
    }

    return merged;
}

/*---------------------------------------------------------------
 Routine : order_fail_data
 Purpose : orders fail data into a list increasing in length
---------------------------------------------------------------*/
Fnode *
  order_fail_data( Fnode *list)
{
    Fnode *p;
    Fnode *local_p;

    Fnode  *local_High = NULL;     /* high end of the order list           */
    int     local_nfm = nfm;       /* next place in rank array             */

    Fnode *ordered = NULL;
    int    max_bit_count;
    int i;

    max_bit_count = list->b->n;

    for(i = 1; i <= max_bit_count; i++) 
    {
        p = list;
        while(p)
        {
            local_p = p;
            p = p->next;

            if(BitCount(local_p->b) == i) 
            {
                if(list == local_p)
                {
                    list = local_p->next;
                }
                unlink_node(local_p);
                /* remove internal links */
                local_p->next = NULL;
                local_p->prev = NULL;

                /* remember last in list */
                if(!ordered)
                {
                    local_High = local_p;
                }
                add_fnode(local_p, &ordered);
            }
        }
    }

    High = local_High;
    Low = ordered;
    nfm = local_nfm;

    return ordered;
}


/*---------------------------------------------------------------
 Routine : compress_fail_data2
 Purpose : Compress the data to minimal sets
---------------------------------------------------------------*/
void
  compress_fail_data2( Fnode **h, int *n, int *oth )
{
    Fnode *p, *q;
    int flag;

    order_fail_data(Low);

/*
 *  for each failure mode, p, in descending order
 */
    for (p = High; p != NULL; p = p->prev) {

        flag = 0;      /* unset flag                                          */
    /*
     *  Compare with each of the failure modes, q, we have already checked.
     */
        for (q = p; q != NULL; q = q->next) {

            if (q != p &&
                q->n != 0) {
                switch( FnodeSubset(q, p) ) {
    
                case SUBSET : /* q absorbs p, set flag and add p's score to q */
                    flag = 1;
                    q->n += p->n;
                    break;
    
                case SUPERSET : /* p absorbs q, unlink q and add score to p  */
                    /*
                    p->n += q->n;
                    unlink_node(q);
                    */
                    break;
        
                default :       /* continue to the next q */
                    break;
                }
            }
        }

     /* If flag set, unlink p */
        if (flag) {
            unlink_node(p);
        }
    }

    *h   = High;
    *n   = nfm;
    *oth = nother;

} /* compress_fail_data */


/*---------------------------------------------------------------
 Routine : compress_fail_data
 Purpose : Compress the data to minimal sets
---------------------------------------------------------------*/
void
  compress_fail_data( Fnode **h, int *n, int *oth )
{
    Fnode *p, *q;
    int flag;

/*
 *  for each failure mode, p, in descending order
 */
    for (p = High; p != NULL; p = p->prev) {

        flag = 0;      /* unset flag                                          */
    /*
     *  Compare with each of the failure modes, q, we have already checked.
     */
        for (q = High; q != p; q = q->prev) {

            if (q->n != 0) {
                switch( FnodeSubset(q, p) ) {
    
                case SUBSET : /* q absorbs p, set flag and add p's score to q */
                    flag = 1;
                    q->n += p->n;
                    break;
    
                case SUPERSET : /* p absorbs q, unlink q and add score to p  */
                    p->n += q->n;
                    unlink_node(q);
                    break;
        
                default :       /* continue to the next q */
                    break;
                }
            }
        }

     /* If flag set, unlink p */
        if (flag) {
            unlink_node(p);
        }
    }

    *h   = High;
    *n   = nfm;
    *oth = nother;

} /* compress_fail_data */

/*---------------------------------------------------------------
 FOR TEST PURPOSES ONLY vvvvvvvvvvvvvvvvvvvvv
---------------------------------------------------------------*/
#ifdef TEST
/*
 * MAIN program to test the module
 *
 */
main()
{
    BitArray *b[20];
    int i;

    for(i = 0; i < 20; i++) {
        b[i] = BitCreate(5);
        BitSetInt(b[i], i);
    }

    initialise_failures(10);

    record_failure(b[10]);
    record_failure(b[1]);
    record_failure(b[10]);
    record_failure(b[3]);
    record_failure(b[10]);
    record_failure(b[10]);
    record_failure(b[3]);
    record_failure(b[11]);
    record_failure(b[11]);
    record_failure(b[10]);
    record_failure(b[2]);
    record_failure(b[11]);

    for(i = 0; i < 20; i++) {
        record_failure(b[i]);
    }

    record_failure(b[8]);

    tree_print(Failtree);
    list_print();

    tree_destroy(Failtree);
}
#endif
