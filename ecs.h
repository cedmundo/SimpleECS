//
// Created on 1/24/24.
//

#ifndef SIMPLEECS_ECS_H
#define SIMPLEECS_ECS_H

#include <stddef.h>
#include <stdbool.h>

#define ECS_ALIVE_FLAG_ID 0

typedef unsigned EntityID;
typedef unsigned FlagID;
typedef unsigned ComponentID;

typedef struct {
    unsigned count;
    unsigned cap;
    EntityID *results;
} QueryResult;

void ECSInit(int componentCount, ...);
void ECSFree();

EntityID ECSCreateEntity();
void ECSDeleteEntity(EntityID entityId);

void *ECSGet(EntityID entityId, ComponentID componentId);
bool ECSHas(EntityID entityId, ComponentID componentId);
void ECSAdd(EntityID entityId, ComponentID componentId, void *data);
void ECSRemove(EntityID entityId, ComponentID componentId);

bool ECSHasFlag(EntityID entityId, FlagID flagId);
void ECSSetFlag(EntityID entityId, FlagID flagId);
void ECSUnsetFlag(EntityID entityId, FlagID flagId);

#endif //SIMPLEECS_ECS_H
