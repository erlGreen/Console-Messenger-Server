#include <stdlib.h>
#include "listlib.h"

struct list* newList() {
    struct list* head = (struct list*) malloc(sizeof(struct list));
    head->next = NULL;
    head->previous = NULL;
    head->value = NULL;
	return head;
}

void addToList(struct list* head, void *element) {
    struct list *iterator = head;
    while(iterator->next != NULL) {
        iterator = iterator->next;
    }
    iterator->next = (struct list*)malloc(sizeof(struct list));
    iterator->next->previous = iterator;
    iterator->next->next = NULL;
    iterator->next->value = element;
}
void remFromList(struct list* head, void *element) {
    struct list *iterator = head;
    while(iterator->next != NULL) {
        if (iterator->next->value == element) {
            struct list *help = iterator->next;
            iterator->next->next->previous = iterator;
            iterator->next = iterator->next->next;
            free(help);
            return;
        }
        iterator = iterator->next;
    }
}

void deleteList(struct list* head) {
    struct list *iterator = head;
    while (iterator->next != NULL) {
        iterator = iterator->next;
        free(iterator->previous);
    }
    free(iterator);
}