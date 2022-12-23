#include "../TestFramework.h"

TEST_FILE

TEST("ExampleTest1") {
    ASSERT(5 == 5);
}

TEST("ExampleTest2") {
    int a = 0;
    ++a;
    ASSERT(a);
}

TEST("ExampleTest3") {
    int b = 0;
    --b;
    ASSERT(b < 0);
}


END_TEST_FILE
