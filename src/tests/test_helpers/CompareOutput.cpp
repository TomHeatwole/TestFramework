#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "../../PrintHelpers.h"

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

// True for lines like "-----Test Output-----"
bool isDashBlock(std::string& line) {
    if (line.size() <= 4) {
        return false;
    }
    for (size_t i = 0; i < 4; ++i) {
        if (line[i] != '-') {
            return false;
        }
    }

    return true;
}

std::string expandColor(std::string& in) {
    if (in[in.size() - 1] != '%') {
        return in;
    }

    auto idx = in.find('%');
    std::string color = in.substr(idx + 1, in.size() - idx - 2);
    std::string line = in.substr(0, idx);

    if (color == "RED") {
        return print::red(line);
    }
    if (color == "BOLD_RED") {
        return print::boldRed(line);
    }
    if (color == "GREEN") {
        return print::green(line);
    }
    if (color == "BOLD_GREEN") {
        return print::boldGreen(line);
    }
    if (color == "YELLOW") {
        return print::yellow(line);
    }
    if (color == "BOLD_YELLOW") {
        return print::boldYellow(line);
    }

    // Shouldn't fall through but I'll leave this here in case
    return in;
}

int exitAndPrint(std::string message, OutputPtr expected, OutputPtr actual) {
    std::cout << message << std::endl << std::endl;
    std::cout << "Expected output:" << std::endl;
    for (auto& line : *expected) {
        std::cout << "    " << expandColor(line) << std::endl;
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

    // Parse the expected output into sets of groups of lines based on
    // indentation. Map topLevel line --> indented lines
    std::unordered_map<std::string, std::vector<std::string>> groups;
    std::string topLevel = expected->at(0);
    std::vector<std::string> indented;
    for (size_t i = 1; i < expected->size(); ++i) {
        std::string line = expected->at(i);
        // Indented
        if (line[0] == ' ') {
            indented.push_back(expandColor(line));
            continue;
        }

        // Need to treat ------ output blocks as indented lines
        if (isDashBlock(line)) {
            indented.push_back(expandColor(line));

            // Capture all lines until the next dash block
            while (++i < expected->size()) {
                line = expected->at(i);
                indented.push_back(expandColor(line));
                if (isDashBlock(line)) {
                    break;
                }
            }

            // Now we've added all these lines as if they were indented.
            // We're ready to process the next line as normal.
            continue;
        }

        // Otherwise we're ready to add a group and start the next one
        groups.emplace(expandColor(topLevel), std::move(indented));
        indented = std::vector<std::string>();
        topLevel = line;
    }
    groups.emplace(expandColor(topLevel), std::move(indented));

    size_t actualPtr = 0;
    std::unordered_set<std::string> actualTopLevelLinesFound;
    while (actualPtr < actual->size()) {
        std::string& topLevel = actual->at(actualPtr);
        actualTopLevelLinesFound.insert(topLevel);
        if (!groups.count(topLevel)) {
            return exitAndPrint(
                    build_string({
                        "Got unexpected line in output: \"",
                        topLevel,
                        "\""}),
                    std::move(expected),
                    std::move(actual));
        }
        const auto& indented = groups.at(topLevel);
        for (size_t i = 0; i < indented.size(); ++i) {
            if (actualPtr == actual->size() - 1) {
                return exitAndPrint(
                        build_string({
                            "Output ended before expected line: \"",
                            indented.at(i),
                            "\""}),
                        std::move(expected),
                        std::move(actual));
            }
            if (indented.at(i) != actual->at(++actualPtr)) {
                return exitAndPrint(
                        build_string({
                            "Unexpected line: \"",
                            actual->at(actualPtr),
                            "\"\n    shortly after \"",
                            topLevel,
                            "\"\n Expected: \"",
                            indented.at(i),
                            "\""}),
                        std::move(expected),
                        std::move(actual));
            }
        }
        ++actualPtr;
    }

    if (actualTopLevelLinesFound.size() < groups.size()) {
        std::stringstream message;
        message << "Didn't find the following expected lines in output:\n";
        std::for_each(
                groups.begin(),
                groups.end(),
                [&actualTopLevelLinesFound, &message](
                        std::pair<std::string, std::vector<std::string>> pair) {
            if (!actualTopLevelLinesFound.count(pair.first)) {
                message << "    " << pair.first << std::endl;
                for (const auto& line : pair.second) {
                    message << "    " << line << std::endl;
                }
            }
        });

        return exitAndPrint(message.str(), std::move(expected), std::move(actual));
    }

    // Passed successfully
    return 0;
}
