#include <stdlib.h>
#include <stdbool.h>

#include "list.h"


List* List_create() {
    List* list = malloc(sizeof(List));
    list->head = NULL;
    list->size = 0;
    return list;
}


void List_add(List* list, int value) {
    ListNode* new = malloc(sizeof(List));
    new->value = value;
    new->next = list->head;
    list->head = new;
    list->size++;
}


void List_insert(List* list, ListNode* node, int value) {
    ListNode* new = malloc(sizeof(List));
    new->value = value;
    new->next = node->next;
    node->next = new;
    list->size++;
}


void List_remove(List* list, int value) {
    ListNode* prev = NULL;
    for (ListNode* current = list->head; current; current = current->next) {
        if (current->value == value) {
            if (current == list->head) {
                list->head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            list->size--;
            break;
        }
        prev = current;
    }
}


void List_clear(List* list) {
    ListNode* current = list->head;
    while (current) {
        ListNode* next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
    list->size = 0;
}


void List_delete(List* list) {
    List_clear(list);
    free(list);
}


bool List_find(List* list, int value) {
    for (ListNode* current = list->head; current; current = current->next) {
        if (current->value = value) {
            return true;
        }
    }
    return false;
}
