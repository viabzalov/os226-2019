#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>	
#include "sched.h"

typedef struct app_info {
	int id;
	int cnt;
} info_t;

typedef struct app {
	void *func;
	info_t* info;
	int priority;
	int deadline;
	int time_lock;
	struct app *next;
} app_t;

app_t *shedule;

enum {
	fifo
	, priority
	, deadline
} policy;

char policy_type[][255] = {
	"fifo"
	, "priority"
	, "deadline"
};

int time = 0;

_Bool cmp_id(app_t *fi, app_t *se) {
	return fi -> info -> id < se -> info -> id;
}

_Bool cmp_priority(app_t *fi, app_t *se) {
	if (fi -> priority == se -> priority) return cmp_id(fi, se);
	return fi -> priority < se -> priority;
}

_Bool cmp_deadline(app_t *fi, app_t *se) {
	if (fi -> deadline == se -> deadline) return cmp_priority(fi, se);
	return fi -> deadline < se -> deadline;
}

_Bool cmp(app_t *fi, app_t *se) {
	switch (policy) {
		case fifo: return cmp_id(fi, se); break;
		case priority: return cmp_priority(fi, se); break;
		case deadline: return cmp_deadline(fi, se); break;
	}
}

void push(app_t *new) {
	app_t *prev = NULL;
	app_t *cur = shedule;
	while (cur && cmp(cur, new)) {
		prev = cur;
		cur = cur -> next;
	}
	if (NULL == prev) {
		new -> next = shedule;
		shedule = new;
		return;
	}
	if (NULL == cur) {
		prev -> next = new;
		return;
	} 
	new -> next = cur -> next;
	cur -> next = new;
}

void sched_new(void (*entrypoint)(void *aspace)
				, void *aspace
				, int priority
				, int deadline) {
	app_t *new = (app_t*)malloc(sizeof(app_t));
	new -> func = entrypoint;
	new -> info = aspace;
	new -> priority = priority;
	if (deadline == -1) {
		deadline = INT_MAX;
	}
	new -> deadline = deadline;
	new -> time_lock = 0;
	new -> next = NULL;
	push(new);
}

void sched_cont(void (*entrypoint)(void *aspace)
				, void *aspace
				, int timeout) {
	app_t *cur = shedule;
	while (cur -> info -> id != ((info_t*)aspace) -> id) {
		cur = cur -> next;
	}
	cur -> time_lock = time + timeout;
}

void sched_time_elapsed(unsigned amount) {
	time += amount;
}

void sched_set_policy(const char *name) {
	for (policy = fifo; policy <= deadline; ++policy) {
		if (!strcmp(policy_type[policy], name)) {
			return;
		}
	}
}

void sched_run(void) {
	while (NULL != shedule) {
		app_t *prev = NULL;
		app_t *cur = shedule;
		while (cur && cur -> time_lock > time) {
			prev = cur;
			cur = cur -> next;
		}
		if (NULL == cur) {
			sched_time_elapsed(1);
			continue;
		} else {
			if (cur -> info -> cnt <= 0) {
				if (prev == NULL) {
					shedule = shedule -> next;
				} else {
					prev -> next = cur -> next;
				}
				continue;
			}
			void (*run)(void*) = cur -> func;
			(*run)(cur -> info);
		}
	}
}
