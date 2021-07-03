#ifndef LINKED
#define LINKED
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#ifdef WIN32
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS
#define EMPTY 0xCDCDCDCD
#endif

typedef struct Node 
{
	void* data;
	struct Node* next;
	int priority;
} Node;

Node* initialize_node();

void node_set_data(Node* n, void* data, int priority);

typedef struct List 
{
	Node* head;
	Node* tail;
	unsigned int length;
	bool ordered;
} List;

List* initialize_list();

void add_to_list(List* l, void* data, int priority);

void delete_from_list(List* l, Node* n);

void free_list_nodes(List* list);

void sort_list(List* list);

void swap_nodes(Node* a, Node* b);

void update_tail(List* list);

#endif
