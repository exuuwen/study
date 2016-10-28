#ifndef _lib_list_h
#define _lib_list_h

typedef struct List List;

struct List {
  void*      datum;
  List* prev;
  List* next;
};


typedef void  (*list_proc)(void *);

/* -----------------------------------------------------------------------------
 * Constructors:                                                              */

/* Creates a new empty list. */
List* lib_list_create(void);

/* -----------------------------------------------------------------------------
 * Destructors:                                                               */

/* All elements in list are freed with the Destroy procedure. */
void  lib_list_destroy(List* head, list_proc destroy);

/* -----------------------------------------------------------------------------
 * Selectors:                                                                 */

List* lib_list_first(List* head);
List* lib_list_last(List* head);
List* lib_list_next(List* list);
List* lib_list_prev(List* list);
void* lib_list_peek(List* list);

/* -----------------------------------------------------------------------------
 * Modifiers:                                                                 */

/* Inserts and removes an element anywhere in the list. */
/* Returns the list starting with the newly inserted element. */
List* lib_list_insert(List* list, void* datum);
/* Removes an element and returns the element to the right. */
List* lib_list_remove(List* list);

/* These functions should only be performed on the head of a list. */
List* lib_list_insertFirst(List* head, void* d);
List* lib_list_insertLast(List* head, void* d);
List* lib_list_removeFirst(List* head);
List* lib_list_removeLast(List* head);
List* lib_list_append(List* head1, List* head2);

/* -----------------------------------------------------------------------------
 * Predicates:                                                                */

int   lib_list_isEmpty(List* head);

/* -----------------------------------------------------------------------------
 * Miscellaneous:                                                             */

/* The Print procedure is used to print each element. */
void  lib_list_print(List* head, list_proc print);

int   lib_list_length(List* head);

#endif /* _lib_list_h */

