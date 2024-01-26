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

// Initializes the global state with a fixed size of components
void ECSInit(int componentCount, ...);

// Releases all memory used by the global state
void ECSFree();

// Create or recycle an ECSEntityID
ECSEntityID ECSCreateEntity();

// Clear all flags and component mask for the entityID and pushes the ID into recycle bin
void ECSDeleteEntity(ECSEntityID entityId);

// Get a component from an entity
void *ECSGet(ECSEntityID entityId, ECSComponentID componentId);

// Check if an entity has a component
bool ECSHas(ECSEntityID entityId, ECSComponentID componentId);

// Set component mask for the entity and copy data into the entity's space
void ECSAdd(ECSEntityID entityId, ECSComponentID componentId, void *data);

// Remove a component from an entity (just clear the component mask)
void ECSRemove(ECSEntityID entityId, ECSComponentID componentId);

// Check if entity has a flag
bool ECSHasFlag(ECSEntityID entityId, ECSFlagID flagId);

// Set a flag of an entity
void ECSSetFlag(ECSEntityID entityId, ECSFlagID flagId);

// Clear the flag of an entity
void ECSUnsetFlag(ECSEntityID entityId, ECSFlagID flagId);

// Create a mask of the components or flags given as arguments
ECSMask ECSFilter(int n, ...);

// Run a query with component and flag masks.
// Warning: since this is a very simple implementation all queries are stored in the global state by default,
//  do not execute several queries at the same time since results will be overriden.
QueryResult *ECSRunQuery(ECSMask components, ECSMask flags);

#endif //SIMPLEECS_ECS_H
