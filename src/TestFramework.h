#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

typedef void(*voidFunc)();
typedef std::map<std::string, void(*)()> Tests;

class TestFramework {
  public:
    static void doNothing() {}
    static void executeTests(Tests&& tests) {
        // TODO: Make this like an actual test framework spinning up test
        // environment, parallelizing operations, pretty printing results,
        // etc. instead of just executing test functions in series.
        std::for_each(tests.begin(), tests.end(),
                [](std::pair<std::string, void(*)()> test) {
            std::cout << "Executing: " << test.first << std::endl;
            test.second();
        });
    }
};

class TestMap {
  public:
    Tests&& getTests() {
        return std::move(data_);
    }

    void emplace(std::string name, voidFunc&& func) {
        if (data_.count(name)) {
            throw std::runtime_error("Duplicate test name: " + name);
        }

        data_.emplace(name, func);
    }
  private:
    Tests data_;
};

/*
 * Here's the macro magic that makes the test framework work.
 * Test files should start with TEST_FILE, followed by each test having
 * TEST(name) as a signature and END_TEST_FILE at the end.
 * Under the hood, we make a getTests_ function which writes each user-defined
 * TEST into a map of functions. Then we construct a main method that fetches
 * those tests and passes them to the TestFramework::executeTests processor.
 */

// Create test class and start writing executeTests_ function
#define TEST_FILE class TestClass_ { \
  public: \
    static TestMap getTestMap() { \
        TestMap tests_; \
        TestFramework::doNothing(

#define TEST(name) ); tests_.emplace(name, []()

#define END_TEST_FILE ); \
    return std::move(tests_); \
} \
}; \
int main() { \
    try { \
        TestFramework::executeTests(TestClass_::getTestMap().getTests()); \
    } catch (std::exception& e) { \
        std::cout << "Tests did not execute properly, with error:\n    " \
            << e.what() << std::endl; \
        return 1; \
    } \
    return 0; \
}


