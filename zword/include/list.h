#pragma once

typedef struct ListNode {
    int value;
    struct ListNode* next;
} ListNode;

typedef struct {
    ListNode* head;
    int size;
} List;

List* List_create();

void List_add(List* list, int value);

void List_remove(List* list, int value);

void List_delete(List* list);
