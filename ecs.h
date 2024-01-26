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

typedef unsigned ECSEntityID;
typedef unsigned ECSFlagID;
typedef unsigned ECSComponentID;
typedef unsigned ECSMask;

typedef struct {
    unsigned count;
    unsigned cap;
    ECSEntityID *entities;
} QueryResult;

void ECSInit(int componentCount, ...);
void ECSFree();

ECSEntityID ECSCreateEntity();
void ECSDeleteEntity(ECSEntityID entityId);

void *ECSGet(ECSEntityID entityId, ECSComponentID componentId);
bool ECSHas(ECSEntityID entityId, ECSComponentID componentId);
void ECSAdd(ECSEntityID entityId, ECSComponentID componentId, void *data);
void ECSRemove(ECSEntityID entityId, ECSComponentID componentId);

bool ECSHasFlag(ECSEntityID entityId, ECSFlagID flagId);
void ECSSetFlag(ECSEntityID entityId, ECSFlagID flagId);
void ECSUnsetFlag(ECSEntityID entityId, ECSFlagID flagId);

ECSMask ECSFilter(int n, ...);
QueryResult *ECSRunQuery(ECSMask components, ECSMask flags);

#endif //SIMPLEECS_ECS_H
