#include "list.h"

List* new_list() {
	List* result = (List*)malloc(sizeof(List));
	result->count = 0;
	result->element = NULL;
	result->next = NULL;
	return result;
}

bool list_add(List* list, void* element) {
	if (list != NULL && element != NULL) {
		List* current = list;
		List* prev = current;
		while (current != NULL) {
			prev = current;
			current = current->next;
		}
		memcpy(&prev->element, &element, sizeof(element));
		prev->next = new_list();
		list->count++;
		return true;
	}
	return false;
}

List* list_find(List* list, uint(*idGetter)(void*), uint targetId) {
	if (list != NULL) {
		List* current = list;
		//Iterate until we find the id
		while (current->element != NULL) {
			void* el = current->element;
			uint id = idGetter(el);
			if (id == targetId) {
				return current;
			}
			current = current->next;
		}
	}
	return NULL;
}

bool list_contains(List* list, void* element) {
	List* current = list;
	while (current->element != NULL) {
		if (memcmp(current->element, element, sizeof(element)) == 0) {
			return true;
		}
	}
	return false;
}

bool list_delete(List** list, List* target) {
	if ((*list)->count == 0) {
		return false;
	}
	List* current = *list;
	List* prev = NULL;
	while (current != NULL) {
		if (memcmp(current, target, sizeof(List)) == 0) {
			(*list)->count--;
			if (prev == NULL) {
				if ((*list)->count == 0) {
					*list = new_list();
					return true;
				}
				uint count = (*list)->count;
				*list = current->next;
				(*list)->count = count;
				return true;
			}
			//Change the next list ptr of the previous one to the next one
			prev->next = current->next;
			return true;
		}
		prev = current;
		current = current->next;
	}
	return false;
}

void list_free(List* list) {
	List* current = list;
	while (current != NULL) {
		List* n = current->next;
		free(current);
		current = n;
	}
}

bool list_is_empty(List* list) {
	return list->count == 0;
}
