#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/queue.h>
#include <string.h>

struct event
{
	int num;
	TAILQ_ENTRY(event) next;
};

//TAILQ_HEAD(eventlist, event) list;
TAILQ_HEAD(eventlist, event);
struct eventlist list;

int main()
{
	int ret, i;
	struct event *events_insert[10];
    struct event *tmp;

	TAILQ_INIT(&list);
	for (i=0; i<10; i++)
	{
		events_insert[i] = malloc(sizeof(struct event));
		events_insert[i]->num = i;
		if (i%2)
			TAILQ_INSERT_TAIL(&list, events_insert[i], next);	
		else
			TAILQ_INSERT_HEAD(&list, events_insert[i], next);
	}

	printf("forward:");
	TAILQ_FOREACH(tmp, &list, next)
		printf("%d ", tmp->num);
	printf("\n");

	printf("reverse:");
	TAILQ_FOREACH_REVERSE(tmp, &list, eventlist, next)
		printf("%d ", tmp->num);
	printf("\n");

	for (i=0; i<10; i++)
	{
		if (i%3 == 0)
			TAILQ_REMOVE(&list, events_insert[i], next);
	}

	printf("forward dele 3 mod:");
	TAILQ_FOREACH(tmp, &list, next)
		printf("%d ", tmp->num);
	printf("\n");

	for (i=0; i<10; i++)
		free(events_insert[i]);

	return 0;
}

