#include "List.h"

Node* initialize_node()
{
	Node* node = malloc(sizeof(Node));
	node->data = NULL;
	node->priority = 0;
	return node;
}

void node_set_data(Node* n, void* data, int priority)
{
	memcpy(&n->data, &data, sizeof(data));
	n->priority = priority;
	n->next = initialize_node();
}

List* initialize_list()
{
	List* list = malloc(sizeof(List));
	list->head = initialize_node();
	list->tail = list->head;
	list->length = 0;
	list->ordered = true;
	return list;
}

void add_to_list(List* l, void* data, int priority)
{
	Node* iteration = l->head;
	while (iteration->data != NULL)
	{
		iteration = iteration->next;
	}
	node_set_data(iteration, data, priority);
	l->length++;
	l->tail = iteration;
	if (l->ordered)
	{
		sort_list(l);
	}
}

void delete_from_list(List* l, Node* n)
{
	if (l->length < 1)
	{
		return;
	}
	Node* last = NULL;
	Node* iteration = l->head;
	
	while (iteration->data != NULL)
	{
		int diff = memcmp(iteration, n, sizeof(Node));
		if (diff == 0)
		{
			// Delete the node here
			l->length--;
			if (last != NULL)
			{
				last->next = iteration->next;
				break;
			}
			l->head = iteration->next;
			break;
		}
		last = iteration;
		iteration = iteration->next;
	}
}

void free_list_nodes(List* l) {
	Node* iteration = l->head;
	while (iteration != NULL && iteration->data != NULL)
	{
		#ifdef WIN32
		if (iteration == EMPTY)
		{
			break;
		}
		#endif

		if (iteration->data != NULL)
		{
			free(iteration->data);
			iteration->data = NULL;
		}
		Node* nextNode = iteration->next;
		Node* toFree = iteration;
		free(iteration);
		iteration = nextNode;
	}
	free(l);
	l = NULL;
}

void sort_list(List* list)
{
	// Bubble sort for linked list from geeks for geeks: https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
	if (list == NULL)
	{
		return;
	}
	int swapped, i;
	Node* ptr1;
	Node* lptr = NULL;
	do
	{
		swapped = 0;
		ptr1 = list->head;

		while (ptr1->next != lptr && ptr1->next->data != NULL)
		{
			#ifdef WIN32
			if (ptr1->next == EMPTY)
			{
				break;
			}
			#endif
			if (ptr1->priority < ptr1->next->priority)
			{
				swap_nodes(ptr1, ptr1->next);
				swapped = 1;
			}
			ptr1 = ptr1->next;
		}
		lptr = ptr1;
	} while (swapped);
	update_tail(list);
}

void swap_nodes(Node* a, Node* b)
{
	void* data = malloc(sizeof(a->data));
	memcpy(&data, &a->data, sizeof(a->data));
	memcpy(&a->data, &b->data, sizeof(b->data));
	memcpy(&b->data, &data, sizeof(data));
	int temp = a->priority;
	a->priority = b->priority;
	b->priority = temp;
}

void update_tail(List* list)
{
	Node* prev = list->head;
	Node* iteration = list->head;
	while (iteration->data != NULL)
	{
		prev = iteration;
		iteration = iteration->next;
	}
	list->tail = prev;
}
