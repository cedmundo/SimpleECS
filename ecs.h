//
// Created on 1/24/24.
//

#ifndef SIMPLEECS_ECS_H
#define SIMPLEECS_ECS_H

#include <stddef.h>
#include <stdbool.h>

#define ECS_ALIVE_FLAG_ID (0)
#define ECS_UNFILTERED ((ECSMask)UINT_MAX)
#define ECS_DEFAULT_FLAGS ((ECSMask) 1)

typedef unsigned EntityID;
typedef unsigned FlagID;
typedef unsigned ComponentID;
typedef unsigned ECSMask;

typedef struct {
    unsigned count;
    unsigned cap;
    EntityID *entities;
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

ECSMask ECSFilter(int n, ...);
QueryResult *ECSRunQuery(ECSMask components, ECSMask flags);

#endif //SIMPLEECS_ECS_H
