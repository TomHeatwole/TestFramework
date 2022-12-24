#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <thread>
#include <sstream>
#include <shared_mutex>
#include <unordered_map>

typedef void(*voidFunc)();
typedef std::map<std::string, void(*)()> Tests;

// For capturing the std::out and std::err during test runs
struct TestPrinter {
  public:
    TestPrinter() : streamMutex_() {};
    std::stringstream* getStreamForThread(std::thread::id id) {
        std::shared_lock r_lock(streamMutex_);
        if (streams.count(id)) {
            return &(streams.at(id));
        }

        return nullptr;
    }

    std::ostream& getStream() {
        const auto this_id = std::this_thread::get_id();
        if (startedTestRun_ && this_id != mainThread_) {
            // See if we already have a stream for this thread
            auto* stream = getStreamForThread(this_id);
            if (stream) {
                return *stream;
            }

            // If we don't we need to lock the writer ability on the lock and
            // add a new stream
            std::unique_lock w_lock(streamMutex_);
            streams.emplace(this_id, std::stringstream());
            return streams.at(this_id);
        }

        return std::cout;
    }

  void startTests(std::thread::id id) {
      startedTestRun_ = true;
      mainThread_ = id;
  }

  private:
    std::unordered_map<std::thread::id, std::stringstream> streams;
    std::shared_mutex streamMutex_;
    std::thread::id mainThread_;
    bool startedTestRun_ = false;
};

class TestFramework {
  public:
    static void assert_(bool condition) {
        if (!condition) {
            // TODO: Create assertion error classes
            // TODO: Improve error messaging
            throw std::runtime_error("Assertion failure.");
        }
    }

    static void doNothing() {}

    explicit TestFramework(Tests&& tests): tests_(tests) {
    }

    static TestPrinter& getPrinter() {
        static TestPrinter printer;
        return printer;
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
        getPrinter().startTests(std::this_thread::get_id());

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
                }
                auto* stream =
                    getPrinter().getStreamForThread(std::this_thread::get_id());
                if (stream) {
                    testOutput.emplace_back("------Test Stdout------");
                    testOutput.push_back(stream->str());
                    testOutput.emplace_back("------------------------");
                }

                printLines(testOutput);
            });
        });

        for (auto& thread : testThreads) {
            thread.join();
        }
    }

    void printLines(std::vector<std::string> lines) {
        std::lock_guard g(printMutex_);
        std::for_each(lines.begin(), lines.end(), [](std::string line) {
            std::cout << line << std::endl;
        });
    }

    void printLine(std::string line) {
        printLines({std::move(line)});
    }

  private:
    Tests tests_;
    std::mutex printMutex_;
    // TODO: Use these to print total success info
    // std::mutex dataMutex_;
    // std::set<std::string> failed_;
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
    static ostream& getPrintStream_() {
        return TestFramework::getPrinter().getStream();
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

#define cout TestFrameworkGlobalHelper_::getPrintStream_()
