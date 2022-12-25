#include <string>
#include <sstream>

namespace assert {

class assertion_error : public std::runtime_error {
  public:
    explicit assertion_error(const std::string& message)
        : std::runtime_error(message) {}
};

void assert_(bool condition, std::string message, std::string customMessage="") {
    if (!condition) {
        if (customMessage.empty()) {
            throw assertion_error(message);
        }

        throw assertion_error(
                message + std::string("\n    Test detail: ") + customMessage);
    }
}

void assertTrue_(bool condition, std::string customMessage="") {
    assert_(condition, std::string("Failed asserting that ")
            + std::to_string(condition) + std::string(" is True."), customMessage);
}

void assertFalse_(bool condition, std::string customMessage="") {
    assert_(!condition, std::string("Failed asserting that ")
            + std::to_string(condition) + std::string(" is False."), customMessage);
}

} // namespace

// Macro magic (see https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments)
#define GET_MACRO(_1,_2,NAME,...) NAME

#define ASSERT_TRUE1(condition) assert::assertTrue_(condition)
#define ASSERT_TRUE2(condition, message) assert::assertTrue_(condition, message)
#define ASSERT_TRUE(...) GET_MACRO(__VA_ARGS__, ASSERT_TRUE2, ASSERT_TRUE1)(__VA_ARGS__)

#define ASSERT_FALSE1(condition) assert::assertFalse_(condition)
#define ASSERT_FALSE2(condition, message) assert::assertFalse_(condition, message)
#define ASSERT_FALSE(...) GET_MACRO(__VA_ARGS__, ASSERT_FALSE2, ASSERT_FALSE1)(__VA_ARGS__)
