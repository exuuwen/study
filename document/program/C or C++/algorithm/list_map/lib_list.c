#ifdef LINUX 
#define assert(x)
#define printf printk
#else
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include "lib_memory.h"
#include "lib_list.h"

#define LIBLIST      "List"
#define LIBLISTENTRY "ListEntry"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Selectors */

void* lib_list_peek(List* list)     { return list->datum; }
List* lib_list_first(List* head) { return head->next;  }
List* lib_list_last(List* head)  { return head->prev;  }
List* lib_list_next(List* list)     { return list->next;  }
List* lib_list_prev(List* list)     { return list->prev;  }

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Modifiers */

List* lib_list_setFirst(List* list, List* first)
{ 
    return list->next = first; 
}

List* lib_list_setLast(List* list, List* last)
{ 
    return list->prev = last;  
}

List* lib_list_setDatum(List* list, void* datum)
{
    list->datum = datum;
    return list;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Predicates */

int lib_list_isEmpty(List* head)
{ 
    return (head->next == head); 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Removes a node in the middle of a list. The node is just unlinked,
	 ie not freed. */
void lib_list_unlink(List* node)
{
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Creates a new empty list. */
List* lib_list_create(void)
{
    List* head = lib_malloc(sizeof(List), LIBLIST);
    assert(head);

    head->next = head;
    head->prev = head;
    head->datum = NULL;

    return head;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Disposes a list. All elements in list are freed */
void lib_list_destroy(List* head, list_proc destroy)
{
    List*  current1;
    List*  current2;

    current1 = lib_list_first(head);

    while(current1 != head) 
    {
        current2 = lib_list_next(current1);
        if (destroy != NULL)
            destroy(current1->datum);
        lib_free(current1, sizeof(List), LIBLISTENTRY);
        current1 = current2;
    }

    lib_free(head, sizeof(List), LIBLIST);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Inserts a node in the anywhere in a list. */
List* lib_list_insert(List* list, void* d)
{
    List* new_node = lib_malloc(sizeof(List), LIBLISTENTRY);
    assert(new_node);

    new_node->datum = d;
    new_node->prev = list->prev;
    new_node->next = list;
    new_node->prev->next = new_node;
    new_node->next->prev = new_node;

    return new_node;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Inserts a node in the anywhere in a list. */
List* lib_list_remove(List* list)
{
    List* n;

    n = lib_list_next(list);

    lib_list_unlink(list);
    lib_free(list, sizeof(List), LIBLISTENTRY);

    return n;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Inserts a new element first in list. Ordo(1) */
List* lib_list_insertFirst(List* head, void* datum)
{
    List* new_node = lib_malloc(sizeof(List), LIBLISTENTRY);
    assert(new_node);

    new_node->datum = datum;
    new_node->prev = head;
    new_node->next = head->next;
    new_node->prev->next = new_node;
    new_node->next->prev = new_node;

    return head;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Inserts a new element last in list. Ordo(1) */
List* lib_list_insertLast(List* head, void* datum)
{
    List* new_node = lib_malloc(sizeof(List), LIBLISTENTRY);
    assert(new_node);

    new_node->datum = datum;
    new_node->next = head;
    new_node->prev = head->prev;
    new_node->prev->next = new_node;
    new_node->next->prev = new_node;

    return head;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Remove first element in list. Fatal error if empty list. Ordo(1) */
List* lib_list_removeFirst(List* head)
{
    List* first;

    if(lib_list_isEmpty(head))
        return head;

    first = lib_list_first(head);
    lib_list_unlink(first);
    lib_free(first, sizeof(List), LIBLISTENTRY);

    return head;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Remove last element in list. Fatal error if empty list. Ordo(1) */ 
List* lib_list_removeLast(List* head)
{
	List* last;

	if(lib_list_isEmpty(head))
	  return head;

	last = lib_list_last(head);
	lib_list_unlink(last);
	lib_free(last, sizeof(List), LIBLISTENTRY);

	return head;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Appends two lists. The second list is freed. Ordo(1) */
List* lib_list_append(List* head1, List* head2)
{
	List *last1, *first2;

	last1  = head1->prev;
	first2 = head2->next;

	last1->next  = first2;
	first2->prev = last1;

	head1->prev = head2;
	head2->next = head1;

	lib_list_unlink(head2);
	lib_free(head2, sizeof(List), LIBLIST);

	return head1;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


void lib_list_print(List* head, list_proc print)
{
    List* curr;
    curr = head->next;

    printf("[ ");
    while(curr != head)
    {
        print(lib_list_peek(curr));
        curr = lib_list_next(curr);
        printf(" ");
    }
    printf("]\n");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int lib_list_length(List* head)
{
    int i;
    List* l;

    i = 0;

    for (l = lib_list_first(head); l != head; l = lib_list_next(l))
        i++;

    return i;
}

#define TEST
#ifdef TEST

#include <stdio.h>

void delete_a(void *value)
{
    lib_free(value, sizeof(int), "test"); 
}

void print_a(void *value)
{
    printf("%d", *(int*)value);
}

int main()
{
    int *a[22];
    int i;

    List *list = lib_list_create();
    printf("list empty:%d, length:%d\n", lib_list_isEmpty(list), lib_list_length(list));

    for (i=0; i<20; i++)
    {
        a[i] = lib_malloc(sizeof(int), "test");
        *a[i] = i;

        if (i%2)
            lib_list_insertFirst(list, a[i]);
        else
            lib_list_insertLast(list, a[i]);
    }

    lib_list_print(list, print_a);
    printf("list empty:%d, length:%d\n", lib_list_isEmpty(list), lib_list_length(list));
    
    lib_list_removeLast(list);
    lib_list_removeFirst(list);

    lib_list_print(list, print_a);
    printf("list empty:%d, length:%d\n", lib_list_isEmpty(list), lib_list_length(list));
    
    List *l;
    for (l=lib_list_first(list); l!=list; l=lib_list_next(l))
    {
        void *data = lib_list_peek(l);
        int d = *(int*)data;
        if (d == 7)
        {
            lib_list_remove(l);
            break;
        }
    }

    for (l=lib_list_last(list); l!=list; l=lib_list_prev(l))
    {
        void *data = lib_list_peek(l);
        int d = *(int*)data;
        if (d == 12)
        {
            a[20] = lib_malloc(sizeof(int), "test");
            *a[20] = 20;
            lib_list_insert(l, a[20]);
            break;
        }
    }

    for (l=lib_list_last(list); l!=list; l=lib_list_prev(l))
    {
        void *data = lib_list_peek(l);
        int d = *(int*)data;
        if (d == 5)
        {
            a[21] = lib_malloc(sizeof(int), "test");
            *a[21] = 21;
            lib_list_insert(l, a[21]);
            break;
        }
    }

    lib_list_print(list, print_a);
    printf("list empty:%d, length:%d\n", lib_list_isEmpty(list), lib_list_length(list));

    lib_list_destroy(list, delete_a);

    return 0;
}

#endif

