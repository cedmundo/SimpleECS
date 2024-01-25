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
    ECSInit(1, sizeof(Example));

    EntityID first = ECSCreateEntity();
    assert_int_not_equal(first, UINT_MAX);

    ECSDeleteEntity(first);

    EntityID second = ECSCreateEntity();
    assert_int_not_equal(first, UINT_MAX);

    assert_int_equal(first, second);
    ECSFree();
}

int main() {
    UNUSED_TYPE(jmp_buf);
    UNUSED_TYPE(va_list);

    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_single_component),
            cmocka_unit_test(test_recycle_entities),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
