#include "../TestFramework.h"

TEST_FILE

TEST("AssertFalse") {
    ASSERT_FALSE(false);
    ASSERT_FALSE(false);
    ASSERT_FALSE(false);
    ASSERT_FALSE(true, "lol");
}

TEST("AssertTrue") {
    ASSERT_TRUE(true);
    ASSERT_TRUE(false);
}

END_TEST_FILE
