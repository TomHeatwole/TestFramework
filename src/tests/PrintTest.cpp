#include "../TestFramework.h"

TEST_FILE

TEST("NoPrintTest") {
    ASSERT_TRUE(true);
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

    std::cerr << "This is error text";
}

TEST("NoPrintTest2") {
    ASSERT_TRUE(true);
}

TEST("PrintTest3") {
    std::cout << "Third print test" << std::endl;
    ASSERT_TRUE(false);
}

TEST("NoPrintTest3") {
    ASSERT_TRUE(false);
}


END_TEST_FILE
