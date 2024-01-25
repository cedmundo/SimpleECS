//
// Created on 1/24/24.
//

#ifndef SIMPLEECS_ECS_H
#define SIMPLEECS_ECS_H

#include <stddef.h>
#include <stdbool.h>

typedef int EntityID;
typedef int ComponentID;
typedef struct {
    unsigned count;
    unsigned cap;
    EntityID *results;
} QueryResult;

void ECSInit(int componentCount, ...);
void ECSFree();

EntityID ECSCreate();
void *ECSGet(EntityID entityId, ComponentID componentId);
bool ECSHas(EntityID entityId, ComponentID componentId);
void ECSAdd(EntityID entityId, ComponentID componentId, void *data);
void ECSRemove(EntityID entityId, ComponentID componentId);
void ECSDelete(EntityID entityId);

#endif //SIMPLEECS_ECS_H
