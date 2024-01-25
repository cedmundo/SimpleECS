// clang-format off
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <cmocka.h>
// clang-format on

#include "ecs.h"

#define UNUSED(x) ((void)x)
#define UNUSED_TYPE(x) ((void *)(x *)0)

typedef struct {
    char name[32];
    int age;
} Example;

static void test_init_free(void **state) {
    UNUSED(state);
    ECSInit(1, sizeof(Example));
    ECSFree();
}

int main() {
    UNUSED_TYPE(jmp_buf);
    UNUSED_TYPE(va_list);

    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_init_free),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
