#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#define MAX_POWER_OF_TWO 32

typedef char ALIGN[64];

typedef union header {

	struct {
		size_t size;
		unsigned is_free;
		union header *next;
        union header *prev;
	} s;

	ALIGN stub;

} header_t;

header_t *head[MAX_POWER_OF_TWO], *tail[MAX_POWER_OF_TWO];

size_t power_of_two(size_t n) {

    size_t res = 1, p = 0;

    while (res < n) {
        res <<= 1;
        ++p;
    }

    return p;

}

header_t *get_free_block(size_t size) {

	header_t *cur = head[power_of_two(size)];

	while (cur && !cur->s.is_free) {
        cur = cur->s.next;
    }

    return cur;

}

void free(void *block) {

    if (!block) {
        return;
    }

    header_t *header = (header_t*)(block) - 1;
	void *program_break = sbrk(0);

	if ((char*)block + header->s.size == program_break) {
        size_t p = power_of_two(header->s.size);
		if (head[p] == tail[p]) {
			head[p] = tail[p] = NULL;
		} else {
            tail[p]->s.prev->s.next = NULL;
			tail[p] = tail[p]->s.prev;
		}
		sbrk(0 - header->s.size - sizeof(header_t));
		return;
	}

	header->s.is_free = 1;
}

void *malloc(size_t size) {

	if (!size) {
        return NULL;
    }

	header_t* header = get_free_block(size);

	if (header) {
		header->s.is_free = 0;
		return (void*)(header + 1);
	}

    size = (1 << power_of_two(size));
    size_t total_size = sizeof(header_t) + size;
    void* block = sbrk(total_size);

	if (block == (void*)(-1)) {
		return NULL;
	}

	header = block;
	header->s.size = size;
	header->s.is_free = 0;
	header->s.next = header->s.prev = NULL;

    size_t p = power_of_two(header->s.size);

	if (!head[p]) {
        head[p] = header;
    }

	if (tail[p]) {
        tail[p]->s.next = header;
        header->s.prev = tail[p];
    }

	tail[p] = header;

	return (void*)(header + 1);

}

void *calloc(size_t num, size_t nsize) {

	if (!num || !nsize) {
        return NULL;
    }
    
    size_t size = num * nsize;

	void *block = malloc(size);

	if (!block) {
        return NULL;
    }

	memset(block, 0, size);

	return block;

}

void* realloc(void* block, size_t size) {

	if (!block || !size) {
        return malloc(size);
    }

    header_t* header = (header_t*)(block) - 1;

	if (header->s.size >= size) {
        return block;
    }

	void* res = malloc(size);

	if (res) {
		memcpy(res, block, header->s.size);
		free(block);
	}

	return res;

}
