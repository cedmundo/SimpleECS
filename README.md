# SimpleECS

An ECS based on ember-ecs (https://gitlab.com/Falconerd/ember-ecs) that extends a little bit the flags
and query operations and use PascalCase for methods and types (I want to use it with raylib).

# Building
```
mkdir cmake-build-debug
cmake -B cmake-build-debug -S .
```

## Usage

This is a quick example of usage:
```c
#include <stdio.h>
#include <ecs.h>

// Components are just structs
typedef struct {
    int number;
} Odd;

typedef struct {
    int number;
} Even;

// Systems are just plain functions
void SytemPrintOdds(ECSComponentID oddCID) {
    ECSQueryResult *result = ECSRunQuery(ECSFilter(1, oddCID), ECS_DEFAULT_FLAGS);
    for (int i=0;i<result->count;i++) {
        ECSEntityID entityId = result->entities[i];
        Odd *odd = ECSGet(entityID, oddCID);
        printf("ODD %d = %d\n", entityId, odd->number);
    }
}

void SytemPrintEvens(ECSComponentID evenCID) {
    ECSQueryResult *result = ECSRunQuery(ECSFilter(1, evenCID), ECS_DEFAULT_FLAGS);
    for (int i=0;i<result->count;i++) {
        ECSEntityID entityId = result->entities[i];
        Even *even = ECSGet(entityID, evenCID);
        printf("EVEN %d = %d\n", entityId, even->number);
    }
}

void SystemPrintTens(ECSFlagID tenFID, ECSFlagID beforeFiftyFID) {
    ECSQueryResult *result = ECSRunQuery(ECS_UNFILTERED, ECSFilter(1, tenFID));
    for (int i=0;i<result->count;i++) {
        ECSEntityID entityId = result->entities[i];
        if (ECSHasFlag(entityId, beforeFiftyFID)) {
            printf("TEN %d (before fifty)\n", entityId);
        }
    }
}

void main() {
    ECSInit();
    ECSFlagID tenFID = ECSRegisterFlag();
    ECSFlagID firstFiftyFID = ECSRegisterFlag();
    ECSComponentID oddCID = ECSRegisterComponent(sizeof(Odd));
    ECSComponentID evenCID = ECSRegisterComponent(sizeof(Even));
    ECSMakeLayout();
    
    for (int i=0;i<100;i++) {
        ECSEntityID entityId = ECSCreateEntity();
        if (i % 2 == 0) {
            ECSAdd(entityId, evenCID, &(Even) {
                .number = i,
            });
        } else {
            ECSAdd(entityId, oddCID, &(Odd) {
                .number = i,
            });
        }

        if (i < 50) {
            ECSSetFlag(entityId, firstFiftyFID);
        }

        if (i % 10 == 0) {
            ECSSetFlag(entityId, tenFID);
        }
    }
    
    SytemPrintOdds(oddCID);
    SytemPrintEvens(evenCID);
    SystemPrintTens(tenFID, firstFiftyFID);
    ECSFree();
}
```

## Dependencies
It uses CMocka for tests, but they can be disabled with `BUILD_TEST=OFF` option.
