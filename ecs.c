//
// Created on 1/24/24.
//

#include "ecs.h"

#include <stdarg.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>
#include <limits.h>

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

    if (state.componentStore.data != NULL) {
        free(state.componentStore.data);
    }
}

EntityID ECSCreateEntity() {
    if (state.deletedEntities.count > 0) {
        unsigned index = --state.deletedEntities.count;
        EntityID recycled = state.deletedEntities.entityIds[index];
        state.deletedEntities.entityIds[index] = 0L;
        return recycled;
    }

    unsigned id = state.entityStore.count++;
    if (state.entityStore.cap == id) {
        unsigned *new_flag_array = reallocarray(state.entityStore.flagArray, state.entityStore.cap * 2, sizeof(unsigned));
        unsigned *new_mask_array = reallocarray(state.entityStore.maskArray, state.entityStore.cap * 2, sizeof(unsigned));
        void *new_data = reallocarray(state.componentStore.data, state.componentStore.cap * 2, state.componentStore.size);
        EntityID *new_results = reallocarray(state.queryResult.entities, state.entityStore.cap * 2, sizeof(EntityID));
        if (new_flag_array == NULL || new_mask_array == NULL || new_data == NULL || new_results == NULL) {
            return UINT_MAX;
        }

        state.entityStore.flagArray = new_flag_array;
        state.entityStore.maskArray = new_mask_array;
        state.entityStore.cap *= 2;

        state.queryResult.cap *= 2;
        state.queryResult.entities = new_results;

        state.componentStore.data = new_data;
        state.componentStore.cap *= 2;
    }

    state.entityStore.maskArray[id] = 0;
    state.entityStore.flagArray[id] = (1 << ECS_ALIVE_FLAG_ID);
    return (EntityID) id;
}

void ECSEntityStackPush(EntityStack *stack, EntityID entityId) {
    if (stack->count + 1 == stack->cap) {
        stack->entityIds = reallocarray(stack->entityIds, stack->cap * 2, sizeof(EntityID));
        stack->cap *= 2;
    }

    stack->entityIds[stack->count] = entityId;
    stack->count++;
}

void ECSDeleteEntity(EntityID entityId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    if (ECSHasFlag(entityId, ECS_ALIVE_FLAG_ID)) {
        state.entityStore.flagArray[entityId] = 0;
        state.entityStore.maskArray[entityId] = 0;
        ECSEntityStackPush(&state.deletedEntities, entityId);
    }
}

void *ECSGet(EntityID entityId, ComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (char*)state.componentStore.data + (entityId * state.componentStore.size + state.componentStore.dataOffsetArray[componentId]);
}

bool ECSHas(EntityID entityId, ComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (state.entityStore.maskArray[entityId] & (1 << componentId)) != 0;
}

void ECSAdd(EntityID entityId, ComponentID componentId, void *data) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.maskArray[entityId] |= (1 << componentId);
    size_t size = state.componentStore.dataSizeArray[componentId];
    void *ptr = ECSGet(entityId, componentId);
    memcpy(ptr, data, size);
}

void ECSRemove(EntityID entityId, ComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.maskArray[entityId] &= ~(1 << componentId);
}

bool ECSHasFlag(EntityID entityId, FlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (state.entityStore.flagArray[entityId] & (1 << flagId)) != 0;
}

void ECSSetFlag(EntityID entityId, FlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.flagArray[entityId] |= (1 << flagId);
}

void ECSUnsetFlag(EntityID entityId, FlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.flagArray[entityId] &= ~(1 << flagId);
}

ECSMask ECSFilter(int n, ...) {
    va_list va;
    unsigned mask = 0;
    va_start(va, n);
    for (int i=0;i<n;i++) {
        mask |= (1 << va_arg(va, ComponentID));
    }
    va_end(va);
    return mask;
}

QueryResult *ECSRunQuery(ECSMask mask, ECSMask flags) {
    state.queryResult.count = 0;
    flags |= (1 << ECS_ALIVE_FLAG_ID);
    for (int i=0; i < state.entityStore.count; i++) {
        if ((state.entityStore.flagArray[i] & flags) == flags && (state.entityStore.maskArray[i] & mask) != 0) {
            state.queryResult.entities[state.queryResult.count++] = i;
        }
    }
    return &state.queryResult;
}