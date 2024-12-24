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
    ECSEntityID *entityIds;
} EntityStack;

typedef struct {
    size_t dataOffsetArray[MAX_COMPONENTS];
    size_t dataSizeArray[MAX_COMPONENTS];
    void *data;
    size_t size;
    unsigned typeCount;
    unsigned flagCount;
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
    ECSQueryResult queryResult;

    ECSComponentID infoCID;
    ECSFlagID aliveFID;
} State;

static State state = { 0 };

void ECSInit() {
    state.aliveFID = ECSRegisterFlag();
    state.infoCID = ECSRegisterComponent(sizeof(ECSInfoComponent));
}

ECSComponentID ECSRegisterComponent(size_t s) {
    assert(state.componentStore.typeCount < MAX_COMPONENTS && "error: maximum number of components reached!");
    size_t componentID = state.componentStore.typeCount;
    state.componentStore.dataSizeArray[componentID] = s;
    state.componentStore.dataOffsetArray[componentID] = state.componentStore.size;

    state.componentStore.size += s;
    state.componentStore.typeCount ++;
    return componentID;
}

ECSFlagID ECSRegisterFlag() {
    return state.componentStore.flagCount++;
}

void ECSMakeLayout() {
    state.deletedEntities.cap = INITIAL_CAPACITY;
    state.deletedEntities.count = 0;
    state.deletedEntities.entityIds = calloc(INITIAL_CAPACITY, sizeof(ECSEntityID));

    state.componentStore.cap = INITIAL_CAPACITY;
    state.componentStore.data = calloc(INITIAL_CAPACITY, state.componentStore.size);

    state.entityStore.count = 0;
    state.entityStore.cap = INITIAL_CAPACITY;
    state.entityStore.flagArray = calloc(INITIAL_CAPACITY, sizeof(unsigned));
    state.entityStore.maskArray = calloc(INITIAL_CAPACITY, sizeof(unsigned));

    state.queryResult.entities = calloc(INITIAL_CAPACITY, sizeof(ECSEntityID));
    state.queryResult.cap = INITIAL_CAPACITY;
}

void ECSFree() {
    if (state.deletedEntities.entityIds != NULL) {
        free(state.deletedEntities.entityIds);
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

    if (state.queryResult.entities != NULL) {
        free(state.queryResult.entities);
    }

    state = (State){0};
}

ECSEntityID ECSCreateEntity(const char *name) {
    if (state.deletedEntities.count > 0) {
        unsigned index = --state.deletedEntities.count;
        ECSEntityID recycled = state.deletedEntities.entityIds[index];
        state.deletedEntities.entityIds[index] = 0L;
        return recycled;
    }

    unsigned id = state.entityStore.count++;
    if (state.entityStore.cap == id) {
        unsigned *new_flag_array = reallocarray(state.entityStore.flagArray, state.entityStore.cap * 2, sizeof(unsigned));
        unsigned *new_mask_array = reallocarray(state.entityStore.maskArray, state.entityStore.cap * 2, sizeof(unsigned));
        void *new_data = reallocarray(state.componentStore.data, state.componentStore.cap * 2, state.componentStore.size);
        ECSEntityID *new_results = reallocarray(state.queryResult.entities, state.entityStore.cap * 2, sizeof(ECSEntityID));
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
    state.entityStore.flagArray[id] = (1 << state.aliveFID);

    ECSInfoComponent info = {0};
    if (name != NULL) {
        size_t name_len = strlen(name);
        name_len = name_len >= ECS_MAX_NAME_SIZE - 1 ? ECS_MAX_NAME_SIZE - 1 : name_len;
        if (name_len > 0) {
            memcpy(info.name, name, name_len);
        }
    }
    ECSAdd(id, state.infoCID, &info);
    return (ECSEntityID) id;
}

ECSEntityID ECSDuplicate(ECSEntityID entityId) {
    ECSEntityID duplicateId = ECSCreateEntity(NULL);

    // Copy data
    memcpy(ECSGet(duplicateId, state.infoCID), ECSGet(entityId, state.infoCID), state.componentStore.size);

    // Set flags
    state.entityStore.flagArray[duplicateId] = state.entityStore.flagArray[entityId];
    state.entityStore.maskArray[duplicateId] = state.entityStore.maskArray[entityId];
    return duplicateId;
}

void ECSEntityStackPush(EntityStack *stack, ECSEntityID entityId) {
    if (stack->count + 1 == stack->cap) {
        stack->entityIds = reallocarray(stack->entityIds, stack->cap * 2, sizeof(ECSEntityID));
        stack->cap *= 2;
    }

    stack->entityIds[stack->count] = entityId;
    stack->count++;
}

void ECSDeleteEntity(ECSEntityID entityId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    if (ECSHasFlag(entityId, state.aliveFID)) {
        state.entityStore.flagArray[entityId] = 0;
        state.entityStore.maskArray[entityId] = 0;
        ECSEntityStackPush(&state.deletedEntities, entityId);
    }
}

ECSComponentID ECSGetInfoComponentID() {
    return state.infoCID;
}

void *ECSGet(ECSEntityID entityId, ECSComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (char*)state.componentStore.data + (entityId * state.componentStore.size + state.componentStore.dataOffsetArray[componentId]);
}

bool ECSHas(ECSEntityID entityId, ECSComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (state.entityStore.maskArray[entityId] & (1 << componentId)) != 0;
}

void ECSAdd(ECSEntityID entityId, ECSComponentID componentId, void *data) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.maskArray[entityId] |= (1 << componentId);
    size_t size = state.componentStore.dataSizeArray[componentId];
    void *ptr = ECSGet(entityId, componentId);
    memcpy(ptr, data, size);
}

void ECSRemove(ECSEntityID entityId, ECSComponentID componentId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.maskArray[entityId] &= ~(1 << componentId);
}

ECSFlagID ECSGetAliveFlagID() {
    return state.aliveFID;
}

bool ECSHasFlag(ECSEntityID entityId, ECSFlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    return (state.entityStore.flagArray[entityId] & (1 << flagId)) != 0;
}

void ECSSetFlag(ECSEntityID entityId, ECSFlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.flagArray[entityId] |= (1 << flagId);
}

void ECSUnsetFlag(ECSEntityID entityId, ECSFlagID flagId) {
    assert(entityId < UINT_MAX && "out of memory error: invalid entity id");
    state.entityStore.flagArray[entityId] &= ~(1 << flagId);
}

ECSMask ECSFilter(int n, ...) {
    va_list va;
    unsigned mask = 0;
    va_start(va, n);
    for (int i=0;i<n;i++) {
        mask |= (1 << va_arg(va, ECSComponentID));
    }
    va_end(va);
    return mask;
}

ECSQueryResult *ECSRunQuery(ECSMask mask, ECSMask flags) {
    state.queryResult.count = 0;
    flags |= (1 << state.aliveFID);
    for (int i=0; i < state.entityStore.count; i++) {
        if ((state.entityStore.flagArray[i] & flags) == flags && (state.entityStore.maskArray[i] & mask) == mask) {
            state.queryResult.entities[state.queryResult.count++] = i;
        }
    }
    return &state.queryResult;
}