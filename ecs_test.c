// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "ecs.h"

#include <limits.h>

#define UNUSED(x) ((void)x)
#define UNUSED_TYPE(x) ((void *)(x *)0)

typedef struct {
    char name[32];
    int color;
} Example;

typedef struct {
    int number;
} Odd;

typedef struct {
    int number;
} Even;

static void test_single_component(void **state) {
    UNUSED(state);
    ComponentID exampleCID = 0;
    ECSInit(1, sizeof(Example));

    EntityID entity = ECSCreateEntity();
    assert_int_not_equal(entity, UINT_MAX);

    assert_false(ECSHas(entity, exampleCID));
    ECSAdd(entity, exampleCID, &(Example) {
        .name = "bob",
        .color = 0xFF00FF,
    });
    assert_true(ECSHas(entity, exampleCID));

    Example *stored = ECSGet(entity, exampleCID);
    assert_string_equal(stored->name, "bob");
    assert_int_equal(stored->color, 0xFF00FF);

    ECSRemove(entity, exampleCID);
    assert_false(ECSHas(entity, exampleCID));

    ECSFree();
}

static void test_recycle_entities(void **state) {
    UNUSED(state);

    ECSInit(1, sizeof(Example));

    EntityID first = ECSCreateEntity();
    assert_int_not_equal(first, UINT_MAX);

    ECSDeleteEntity(first);

    EntityID second = ECSCreateEntity();
    assert_int_not_equal(first, UINT_MAX);

    assert_int_equal(first, second);
    ECSFree();
}

static void test_entity_flags(void **state) {
    UNUSED(state);

    ECSInit(1, sizeof(Example));

    FlagID aliveFID = ECS_ALIVE_FLAG_ID;
    FlagID exampleFID = 2;
    EntityID entityId = ECSCreateEntity();
    assert_int_not_equal(entityId, UINT_MAX);
    assert_true(ECSHasFlag(entityId, aliveFID));

    assert_false(ECSHasFlag(entityId, exampleFID));
    ECSSetFlag(entityId, exampleFID);
    assert_true(ECSHasFlag(entityId, exampleFID));
    ECSUnsetFlag(entityId, exampleFID);
    assert_false(ECSHasFlag(entityId, exampleFID));

    ECSDeleteEntity(entityId);
    assert_false(ECSHasFlag(entityId, aliveFID));
    ECSFree();
}

static void test_query(void **state) {
    UNUSED(state);

    ECSInit(2, sizeof(Odd), sizeof(Even), sizeof(Example));
    ComponentID OddCID = 0;
    ComponentID EvenCID = 1;
    ComponentID UnknownCID = 2;
    FlagID TenFID = 1;
    FlagID UnknownFID = 2;
    FlagID FirstFiftyFID = 3;

    for (int i=0;i<100;i++) {
        EntityID entityId = ECSCreateEntity();
        if (i % 2 == 0) {
            ECSAdd(entityId, EvenCID, &(Even) {
                .number = i,
            });
        } else {
            ECSAdd(entityId, OddCID, &(Even) {
                .number = i,
            });
        }

        if (i < 50) {
            ECSSetFlag(entityId, FirstFiftyFID);
        }

        if (i % 10 == 0) {
            ECSSetFlag(entityId, TenFID);
        }
    }

    QueryResult *result;

    result = ECSRunQuery(ECS_UNFILTERED, ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 100);

    result = ECSRunQuery(ECSFilter(1, OddCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 50);

    result = ECSRunQuery(ECSFilter(1, EvenCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 50);

    result = ECSRunQuery(ECSFilter(1, UnknownCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 0);

    result = ECSRunQuery(ECS_UNFILTERED, ECSFilter(1, TenFID));
    assert_int_equal(result->count, 10);

    result = ECSRunQuery(ECSFilter(1, EvenCID), ECSFilter(1, FirstFiftyFID));
    assert_int_equal(result->count, 25);

    result = ECSRunQuery(ECSFilter(1, EvenCID), ECSFilter(1, UnknownFID));
    assert_int_equal(result->count, 0);

    ECSFree();
}

int main() {
    UNUSED_TYPE(jmp_buf);
    UNUSED_TYPE(va_list);

    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_single_component),
            cmocka_unit_test(test_recycle_entities),
            cmocka_unit_test(test_entity_flags),
            cmocka_unit_test(test_query),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
