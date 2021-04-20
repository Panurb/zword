#include <stdlib.h>

#include "list.h"


List* List_create() {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->size = 0;
    return list;
}


void List_append(List* list, int value) {
    ListNode* new = malloc(sizeof(List));
    new->value = value;
    new->next = NULL;

    if (list->head) {
        ListNode* current = list->head;
        while (current->next) {
            current = current->next;
        }
        current->next = new;
    } else {
        list->head = new;
    }
    list->size++;
}


void List_delete(List* list) {
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
