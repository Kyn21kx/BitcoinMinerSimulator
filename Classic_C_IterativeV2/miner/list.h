#ifndef LIST_H
#define LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned int uint;

typedef struct List {
	struct List* next;
	void* element;
	uint count;
} 
List;

List* new_list();

bool list_add(List* list, void* element);

List* list_find(List* list, uint(*idGetter)(void*), uint targetId);

bool list_contains(List* list, void* element);

bool list_delete(List** list, List* target);

void list_free(List* list);

//Note: this is only based on the "count" element, so, it is important that the head is passed correctly
bool list_is_empty(List* list);

#endif