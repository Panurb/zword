#include <stdlib.h>

#include "list.h"


List* List_create(int value) {
    List* head = malloc(sizeof(List));
    head->value = value;
    head->next = NULL;
    return head;
}


void List_append(List* head, int value) {
    List* new = malloc(sizeof(List));
    new->value = value;
    new->next = NULL;

    List* current = head;
    while (1) {
        if (!current->next) {
            current->next = new;
            break;
        }
        current = current->next;
    }
}


void List_delete(List* head) {
    List* current = head;
    while (current) {
        List* next = current->next;
        free(current);
        current = next;
    }
}
