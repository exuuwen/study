#ifndef _QUEUE_H_
#define	_QUEUE_H_
#include "../CommonLib.h"
/*
 * This file defines four types of data structures: singly-linked lists,
 * singly-linked tail queues, lists and tail queues.
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A singly-linked tail queue is headed by a pair of pointers, one to the
 * head of the list and the other to the tail of the list. The elements are
 * singly linked for minimum space and pointer manipulation overhead at the
 * expense of O(n) removal for arbitrary elements. New elements can be added
 * to the list after an existing element, at the head of the list, or at the
 * end of the list. Elements being removed from the head of the tail queue
 * should use the explicit macro for this purpose for optimum efficiency.
 * A singly-linked tail queue may only be traversed in the forward direction.
 * Singly-linked tail queues are ideal for applications with large datasets
 * and few or no removals or for implementing a FIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may be traversed in either direction.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 *
 *
 *			SLIST	LIST	STAILQ	TAILQ
 * _HEAD		+	+	+	+
 * _HEAD_INITIALIZER	+	+	+	+
 * _ENTRY		+	+	+	+
 * _INIT		+	+	+	+
 * _EMPTY		+	+	+	+
 * _FIRST		+	+	+	+
 * _NEXT		+	+	+	+
 * _PREV		-	-	-	+
 * _LAST		-	-	+	+
 * _FOREACH		+	+	+	+
 * _FOREACH_REVERSE	-	-	-	+
 * _INSERT_HEAD		+	+	+	+
 * _INSERT_BEFORE	-	+	-	+
 * _INSERT_AFTER	+	+	+	+
 * _INSERT_TAIL		-	-	+	+
 * _CONCAT		-	-	+	+
 * _REMOVE_HEAD		+	-	+	-
 * _REMOVE		+	+	+	+
 *
 */


/*
 * Tail queue declarations.
 */
#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
}

#define	LIST_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define	LIST_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}

/*
 * Tail queue functions.
 */
#define	LIST_CONCAT(head1, head2, field) do {				\
	if (!LIST_EMPTY(head2)) {					\
		*(head1)->tqh_last = (head2)->tqh_first;		\
		(head2)->tqh_first->field.tqe_prev = (head1)->tqh_last;	\
		(head1)->tqh_last = (head2)->tqh_last;			\
		LIST_INIT((head2));					\
	}								\
} while (0)

#define	LIST_EMPTY(head)	((head)->tqh_first == NULL)

#define	LIST_FIRST(head)	((head)->tqh_first)



#define	LIST_NEXT(elm, field) ((elm)->field.tqe_next)



#define	LIST_FOREACH(var, head, field)					\
	for ((var) = LIST_FIRST((head));				\
	    (var);							\
	    (var) = LIST_NEXT((var), field))
/*
#define	LIST_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
	
#define	LIST_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
	
#define	LIST_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = LIST_LAST((head), headname);			\
	    (var);							\
	    (var) = LIST_PREV((var), headname, field))
*/
#define	LIST_INIT(head) do {						\
	LIST_FIRST((head)) = NULL;					\
	(head)->tqh_last = &LIST_FIRST((head));			\
} while (0)

#define	LIST_INSERT_AFTER(head, listelm, elm, field) do {		\
	if ((LIST_NEXT((elm), field) = LIST_NEXT((listelm), field)) != NULL)\
		LIST_NEXT((elm), field)->field.tqe_prev = 		\
		    &LIST_NEXT((elm), field);				\
	else								\
		(head)->tqh_last = &LIST_NEXT((elm), field);		\
	LIST_NEXT((listelm), field) = (elm);				\
	(elm)->field.tqe_prev = &LIST_NEXT((listelm), field);		\
} while (0)

#define	LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	LIST_NEXT((elm), field) = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &LIST_NEXT((elm), field);		\
} while (0)

#define	LIST_INSERT_HEAD(head, elm, field) do {			\
	if ((LIST_NEXT((elm), field) = LIST_FIRST((head))) != NULL)	\
		LIST_FIRST((head))->field.tqe_prev =			\
		    &LIST_NEXT((elm), field);				\
	else								\
		(head)->tqh_last = &LIST_NEXT((elm), field);		\
	LIST_FIRST((head)) = (elm);					\
	(elm)->field.tqe_prev = &LIST_FIRST((head));			\
} while (0)

#define	LIST_INSERT_TAIL(head, elm, field) do {			\
	LIST_NEXT((elm), field) = NULL;				\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &LIST_NEXT((elm), field);			\
} while (0)



#define	LIST_REMOVE(head, elm, field) do {				\
	if ((LIST_NEXT((elm), field)) != NULL)				\
		LIST_NEXT((elm), field)->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = LIST_NEXT((elm), field);		\
} while (0)


#endif /* !_SYS_QUEUE_H_ */
