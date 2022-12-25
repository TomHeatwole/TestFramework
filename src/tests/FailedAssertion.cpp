#include "../TestFramework.h"

TEST_FILE

TEST("FailedAssertion") {
    ASSERT_TRUE(false, "This really shoulda been true!");
}

END_TEST_FILE
