#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <thread>
#include <sstream>
#include <shared_mutex>
#include <unordered_map>

#include "TestPrinter.h"

typedef void(*voidFunc)();
typedef std::map<std::string, void(*)()> Tests;

class TestFramework {
  public:
    static void assert_(bool condition) {
        if (!condition) {
            // TODO: Create assertion error classes
            // TODO: Improve error messaging
            throw std::runtime_error("Assertion failure.");
        }
    }

    explicit TestFramework(Tests&& tests): tests_(tests) {
    }

    void executeTests() {
        // TODO: Make this like an actual test framework spinning up test
        // environment, parallelizing operations, pretty printing results,
        // etc. instead of just executing test functions in series.
        std::vector<std::thread> testThreads;
        printLine(
                std::string("Executing ") +
                std::to_string(tests_.size()) +
                std::string(" tests:"));
        startTests();
        std::for_each(tests_.begin(), tests_.end(),
                [this, &testThreads]
                (std::pair<std::string, void(*)()> test) mutable {
            // TODO: look into batching or a max thread count w/ semaphore
            testThreads.emplace_back([this, test = std::move(test)]() mutable {
                std::vector<std::string> testOutput;
                try {
                    test.second();
                    testOutput.push_back(test.first + std::string("...OK"));
                } catch (std::exception &e) {
                    testOutput.push_back(test.first + std::string("..."));
                    testOutput.push_back(
                            std::string("    failed with exception: ")
                            + e.what());
                    std::lock_guard lg(dataMutex_);
                    failed_.push_back(test.first);
                }
                auto* outStream =
                    getOutPrinter().getStreamForThread(std::this_thread::get_id());
                if (outStream) {
                    testOutput.emplace_back("------Test Stdout------");
                    testOutput.push_back(outStream->str());
                    testOutput.emplace_back("------------------------");
                }

                auto* errStream =
                    getErrPrinter().getStreamForThread(std::this_thread::get_id());
                if (errStream) {
                    testOutput.emplace_back("------Test Stderr------");
                    testOutput.push_back(errStream->str());
                    testOutput.emplace_back("------------------------");
                }

                printLines(testOutput);
            });
        });

        for (auto& thread : testThreads) {
            thread.join();
        }

        // Print final results
        std::cout << std::endl;
        size_t numTests = tests_.size();
        size_t numFailed = failed_.size();
        if (numFailed == 0) {
            std::cout << "All " << std::to_string(numTests)
                << " tests passed!\n";
        } else {
            std::cout << std::to_string(numTests - numFailed) << " of "
                << numTests << " tests passed.\n";
            std::cout << "The following tests failed:\n";

            // Print faild in consistent order
            std::sort(failed_.begin(), failed_.end());
            for (const auto& name : failed_) {
                std::cout << "    " << name << std::endl;
            }
        }
    }

    static TestPrinter& getOutPrinter() {
        static TestPrinter printer(&std::cout);
        return printer;
    }

    static TestPrinter& getErrPrinter() {
        static TestPrinter printer(&std::cerr);
        return printer;
    }

    static void doNothing() {}

  private:
    void printLines(std::vector<std::string> lines) {
        std::lock_guard g(printMutex_);
        std::for_each(lines.begin(), lines.end(), [](std::string line) {
            std::cout << line << std::endl;
        });
    }

    void printLine(std::string line) {
        printLines({std::move(line)});
    }

    static void startTests() {
        getOutPrinter().startTests(std::this_thread::get_id());
        getErrPrinter().startTests(std::this_thread::get_id());
    }

    Tests tests_;
    std::mutex printMutex_;
    std::mutex dataMutex_;
    std::vector<std::string> failed_;
};

class TestMap {
  public:
    Tests&& getTests() {
        return std::move(data_);
    }

    TestMap(): data_() {
    }

    TestMap(TestMap&& otherMap) {
        data_ = std::move(otherMap.data_);
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

// Putting this in namespace std so we can overwrite what std::out does
namespace std {
struct TestFrameworkGlobalHelper_ {
  public:
    static ostream& getOutStream_() {
        return TestFramework::getOutPrinter().getStream();
    }

    static ostream& getErrStream_() {
        return TestFramework::getErrPrinter().getStream();
    }
};
} // namespace std

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

// Close previous test, open new one
#define TEST(name) ); tests_.emplace(name, []()

// Close previous test, return, add main method for execution
#define END_TEST_FILE ); \
    return std::move(tests_); \
} \
}; \
int main() { \
    try { \
        TestFramework t(TestClass_::getTestMap().getTests()); \
        t.executeTests(); \
    } catch (std::exception& e) { \
        std::cout << "Tests did not execute properly, with error:\n    " \
            << e.what() << std::endl; \
        return 1; \
    } \
    return 0; \
}

#define ASSERT(condition) TestFramework::assert_(condition)

#define cout TestFrameworkGlobalHelper_::getOutStream_()

#define cerr TestFrameworkGlobalHelper_::getErrStream_()
