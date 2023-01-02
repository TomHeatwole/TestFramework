#include "../TestFramework.h"


struct Lol {
    bool operator==(const Lol& otherLol) {
        return false;
    }
};

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

TEST("AssertEqual") {
    ASSERT_EQ(5, 5);
    ASSERT_EQ("asf", "asf");
    ASSERT_EQ(7, 5);
}

END_TEST_FILE
