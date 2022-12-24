#include "../TestFramework.h"

TEST_FILE

/* TODO: Enable once meta test framework is smarter about output
TEST("NoPrintTest") {
    ASSERT(true);
}
*/

TEST("PrintTest") {
    int g = 5;
    std::cout << std::to_string(g);
    std::cout << " six seven" << std::endl;
}


END_TEST_FILE
