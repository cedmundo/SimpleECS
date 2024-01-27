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

    ECSInit();
    ECSComponentID exampleCID = ECSRegisterComponent(sizeof(Example));
    ECSMakeLayout();

    ECSEntityID entityId = ECSCreateEntity("example");
    assert_int_not_equal(entityId, UINT_MAX);

    ECSInfoComponent *info = ECSGet(entityId, ECSGetInfoComponentID());
    assert_string_equal(info->name, "example");

    assert_false(ECSHas(entityId, exampleCID));
    ECSAdd(entityId, exampleCID, &(Example) {
        .name = "bob",
        .color = 0xFF00FF,
    });
    assert_true(ECSHas(entityId, exampleCID));

    Example *stored = ECSGet(entityId, exampleCID);
    assert_string_equal(stored->name, "bob");
    assert_int_equal(stored->color, 0xFF00FF);

    ECSRemove(entityId, exampleCID);
    assert_false(ECSHas(entityId, exampleCID));

    ECSFree();
}

static void test_recycle_entities(void **state) {
    UNUSED(state);

    ECSInit();
    ECSRegisterComponent(sizeof(Example));
    ECSMakeLayout();

    ECSEntityID first = ECSCreateEntity("first");
    assert_int_not_equal(first, UINT_MAX);

    ECSDeleteEntity(first);

    ECSEntityID second = ECSCreateEntity("second");
    assert_int_not_equal(first, UINT_MAX);

    assert_int_equal(first, second);
    ECSFree();
}

static void test_entity_flags(void **state) {
    UNUSED(state);

    ECSInit();
    ECSFlagID aliveFID = ECSGetAliveFlagID();
    ECSFlagID exampleFID = ECSRegisterFlag();
    ECSMakeLayout();

    ECSEntityID entityId = ECSCreateEntity("entity");
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

static void test_query_simple(void **state) {
    UNUSED(state);

    ECSInit();
    ECSComponentID exampleCID = ECSRegisterComponent(sizeof(Example));
    ECSMakeLayout();

    ECSEntityID entityId = ECSCreateEntity(NULL);
    ECSAdd(entityId, exampleCID, &(Example){
        .name = "hello world",
        .color = 0x00FF00,
    });
    ECSQueryResult *result;

    result = ECSRunQuery(ECS_UNFILTERED, ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 1);

    result = ECSRunQuery(ECSFilter(1, exampleCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 1);
    ECSFree();
}

static void test_query_complex(void **state) {
    UNUSED(state);

    ECSInit();
    ECSComponentID oddCID = ECSRegisterComponent(sizeof(Odd));
    ECSComponentID evenCID = ECSRegisterComponent(sizeof(Even));
    ECSComponentID exampleCID = ECSRegisterComponent(sizeof(Example));
    ECSFlagID tenFID = ECSRegisterFlag();
    ECSFlagID unknownFID = ECSRegisterFlag();
    ECSFlagID beforeFiftyFID = ECSRegisterFlag();
    ECSMakeLayout();

    for (int i=0;i<100;i++) {
        ECSEntityID entityId = ECSCreateEntity(NULL);
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
            ECSSetFlag(entityId, beforeFiftyFID);
        }

        if (i % 10 == 0) {
            ECSSetFlag(entityId, tenFID);
        }
    }

    ECSQueryResult *result;

    result = ECSRunQuery(ECS_UNFILTERED, ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 100);

    result = ECSRunQuery(ECSFilter(1, oddCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 50);

    result = ECSRunQuery(ECSFilter(1, evenCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 50);

    result = ECSRunQuery(ECSFilter(1, exampleCID), ECS_DEFAULT_FLAGS);
    assert_int_equal(result->count, 0);

    result = ECSRunQuery(ECS_UNFILTERED, ECSFilter(1, tenFID));
    assert_int_equal(result->count, 10);

    result = ECSRunQuery(ECSFilter(1, evenCID), ECSFilter(1, beforeFiftyFID));
    assert_int_equal(result->count, 25);

    result = ECSRunQuery(ECSFilter(1, evenCID), ECSFilter(1, unknownFID));
    assert_int_equal(result->count, 0);

    ECSFree();
}

static void test_duplicate(void **state) {
    UNUSED(state);

    ECSInit();
    ECSComponentID oddCID = ECSRegisterComponent(sizeof(Odd));
    ECSComponentID evenCID = ECSRegisterComponent(sizeof(Even));
    ECSFlagID dummyFID = ECSRegisterFlag();
    ECSMakeLayout();

    ECSEntityID originalEntityId = ECSCreateEntity(NULL);
    ECSAdd(originalEntityId, oddCID, &(Odd) {0xFA0});
    ECSAdd(originalEntityId, evenCID, &(Even) {0xF0A });
    ECSSetFlag(originalEntityId, dummyFID);

    ECSEntityID duplicateEntityId = ECSDuplicate(originalEntityId);
    assert_int_not_equal(originalEntityId, duplicateEntityId);
    assert_int_not_equal(duplicateEntityId, UINT_MAX);

    Odd *odd = ECSGet(duplicateEntityId, oddCID);
    assert_int_equal(odd->number, 0xFA0);

    Even *even = ECSGet(duplicateEntityId, evenCID);
    assert_int_equal(odd->number, 0xFA0);

    assert_true(ECSHasFlag(duplicateEntityId, dummyFID));
    ECSFree();
}

int main() {
    UNUSED_TYPE(jmp_buf);
    UNUSED_TYPE(va_list);

    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_single_component),
            cmocka_unit_test(test_recycle_entities),
            cmocka_unit_test(test_entity_flags),
            cmocka_unit_test(test_query_simple),
            cmocka_unit_test(test_query_complex),
            cmocka_unit_test(test_duplicate),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
