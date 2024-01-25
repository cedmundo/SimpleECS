//
// Created on 1/24/24.
//

#include "ecs.h"

#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>

#define INITIAL_CAPACITY 32
#define MAX_COMPONENTS 32

typedef struct {
    unsigned count;
    unsigned cap;
    EntityID *entityIds;
} EntityStack;

typedef struct {
    size_t *dataOffsetArray;
    size_t *dataSizeArray;
    void *data;
    size_t size;
    unsigned typeCount;
    unsigned cap;
} ComponentStore;

typedef struct {
    unsigned *maskArray;
    unsigned *flagArray;
    unsigned count;
    unsigned cap;
} EntityStore;

typedef struct {
    EntityStack deletedEntities;
    EntityStore entityStore;
    ComponentStore componentStore;
    QueryResult queryResult;
} State;

static State state = { 0 };

void ECSInit(int componentCount, ...) {
    assert(componentCount < MAX_COMPONENTS && "error: componentCount greater than max components allowed");

    size_t size = 0L;
    size_t sizes[MAX_COMPONENTS];
    size_t offsets[MAX_COMPONENTS];

    va_list ap;
    va_start(ap, componentCount);
    for (int i = 0; i < componentCount; i++) {
        sizes[i] = va_arg(ap, size_t);
        offsets[i] = size;
        size += sizes[i];
    }
    va_end(ap);

    state.deletedEntities.cap = INITIAL_CAPACITY;
    state.deletedEntities.count = 0;
    state.deletedEntities.entityIds = calloc(INITIAL_CAPACITY, sizeof(EntityID));

    state.componentStore.typeCount = componentCount;
    state.componentStore.cap = INITIAL_CAPACITY;
    state.componentStore.data = calloc(INITIAL_CAPACITY, size);
    state.componentStore.dataSizeArray = calloc(INITIAL_CAPACITY, sizeof(size_t));
    state.componentStore.dataOffsetArray = calloc(INITIAL_CAPACITY, sizeof(size_t));
    state.componentStore.size = size;
    memcpy(state.componentStore.dataSizeArray, sizes, componentCount * sizeof(size_t));
    memcpy(state.componentStore.dataOffsetArray, offsets, componentCount * sizeof(size_t));

    state.entityStore.count = 0;
    state.entityStore.cap = INITIAL_CAPACITY;
    state.entityStore.flagArray = calloc(INITIAL_CAPACITY, sizeof(unsigned));
    state.entityStore.maskArray = calloc(INITIAL_CAPACITY, sizeof(unsigned));
}

void ECSFree() {
    if (state.deletedEntities.entityIds != NULL) {
        free(state.deletedEntities.entityIds);
    }

    if(state.componentStore.dataSizeArray != NULL) {
        free(state.componentStore.dataSizeArray);
    }

    if(state.componentStore.dataOffsetArray != NULL) {
        free(state.componentStore.dataOffsetArray);
    }

    if (state.entityStore.flagArray != NULL) {
        free(state.entityStore.flagArray);
    }

    if (state.entityStore.maskArray != NULL) {
        free(state.entityStore.maskArray);
    }
}
