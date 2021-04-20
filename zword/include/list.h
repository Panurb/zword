typedef struct Node {
    int value;
    struct Node* next;
} List;

List* List_create(int value);

void List_append(List* head, int value);

void List_delete(List* head);
