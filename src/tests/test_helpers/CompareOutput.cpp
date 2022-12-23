#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

typedef std::unique_ptr<std::vector<std::string>> OutputPtr;
OutputPtr getLines(char* inStr) {
    auto lines = std::make_unique<std::vector<std::string>>();
    std::stringstream current;
    for (; *inStr != '\0'; inStr++) {
        if (*inStr == '\n') {
            lines->emplace_back(current.str());
            current.str(std::string()); // clear
            continue;
        }
        current << *inStr;
    }

    lines->push_back(std::move(current.str()));
    return lines;
}

std::string build_string(std::initializer_list<std::string> in) {
    std::stringstream out;
    std::for_each(in.begin(), in.end(), [&out](std::string in) {
        out << in;
    });

    return out.str();
}

int exitAndPrint(std::string message, OutputPtr expected, OutputPtr actual) {
    std::cout << message << std::endl << std::endl;
    std::cout << "Expected output:" << std::endl;
    for (const auto& line : *expected) {
        std::cout << "    " << line << std::endl;
    }
    std::cout << "\nActual output:" << std::endl;
    for (const auto& line : *actual) {
        std::cout << "    " << line << std::endl;
    }
    std::cout << std::endl;
    return 1;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        throw std::runtime_error("Expected exactly 2 outputs to comapre");
    }

    // Seriailze both inputs into vectors of lines
    auto expected = getLines(argv[1]);
    auto actual = getLines(argv[2]);

    // first simply check size and fail if non-identical
    if (expected->size() != actual->size()) {
        std::string message = build_string({
                "Output mismatch. Expected ",
                std::to_string(expected->size()),
                " lines but got ",
                std::to_string(actual->size()),
                " lines."}) ;
        return exitAndPrint(
                std::move(message), std::move(expected), std::move(actual));
    }

    // TODO: Account for tests out of order due to parallel runs
    for (size_t i = 0; i < expected->size(); ++i) {
        if (expected->at(i) != actual->at(i)) {
            std::string message = build_string({
                    "Output mistmatch on line ",
                    std::to_string(i + 1),
                    ":\nExpected: \"",
                    expected->at(i),
                    "\"\nReceived: \"",
                    actual->at(i),
                    "\""});
            return exitAndPrint(
                    std::move(message), std::move(expected), std::move(actual));
        }
    }

    // Passed successfully
    return 0;
}
