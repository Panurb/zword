#pragma once

#include <stdbool.h>

#include "component.h"

#define MAX_HEAP_SIZE 1000


typedef struct {
    int array[MAX_HEAP_SIZE];
    int size;
    Component* component;
    int start;
} Heap;

Heap* Heap_create(Component* component);

void Heap_insert(Heap* heap, int key);

int Heap_extract(Heap* heap);

int Heap_find(Heap* heap, int key);
