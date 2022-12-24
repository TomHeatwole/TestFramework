#include "../TestFramework.h"

TEST_FILE

TEST("NoPrintTest") {
    ASSERT(true);
}

TEST("PrintTest") {
    int g = 5;
    std::cout << std::to_string(g);
    std::cout << " six seven" << std::endl;
}

TEST("PrintTest2") {
    std::cout << "test2" << std::endl;

    for (size_t i = 0; i < 26; ++i) {
        std::cout << static_cast<char>('a' + i);
    }
}

TEST("NoPrintTest2") {
    ASSERT(true);
}

TEST("PrintTest3") {
    std::cout << "Third print test" << std::endl;
    ASSERT(false);
}

TEST("NoPrintTest3") {
    ASSERT(false);
}


END_TEST_FILE
