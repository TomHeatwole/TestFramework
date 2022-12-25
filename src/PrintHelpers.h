#include <string>
#include <sstream>
#include <utility>

/* This class contains helper functions for pretty printing test results.
 * Namely, this is for adding color and text decoration wherever needed,
 * abstracting the need to use ASNI color codes outside of this file.
 *
 * More info on the ASNI codes:
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 */

namespace print {

enum TextColor {
    tc_none = 0,
    tc_black = 30,
    tc_red = 31,
    tc_green = 32,
    tc_yellow = 33,
    tc_blue = 34,
    tc_magenta = 35,
    tc_cyan = 36,
    tc_white = 37,
};

enum BackgroundColor {
    bc_none = 0,
    bc_black = 40,
    bc_red = 41,
    bc_green = 42,
    bc_yellow = 43,
    bc_blue = 44,
    bc_magenta = 45,
    bc_cyan = 46,
    bc_white = 47,
};

enum Style {
    s_none = 0,
    s_bold = 1,
    s_underline = 4,
};

std::string decorate(
        std::string&& in,
        TextColor tc = TextColor::tc_black,
        Style s = Style::s_none,
        BackgroundColor bc = BackgroundColor::bc_none) {
    if ((tc | s | bc) == 0) {
        return in;
    }

    std::stringstream out;
    std::stringstream prefix;
    prefix << "\033[";
    bool first = true;
    for (int i : {
            static_cast<int>(tc),
            static_cast<int>(s),
            static_cast<int>(bc)}) {
        if (i > 0) {
            if (!first) {
                prefix << ';';
            }
            prefix << std::to_string(i);
            first = false;
        }
    }

    prefix << 'm';
    size_t prevPtr = 0;
    for (size_t newLinePtr = in.find('\n');
            newLinePtr != std::string::npos;
            newLinePtr = in.find(newLinePtr, '\n')) {
        out << prefix.str() << in.substr(prevPtr, newLinePtr - prevPtr)
            << "\033[0m\n";
        prevPtr = newLinePtr + 1;
    }

    out << prefix.str() << in.substr(prevPtr, in.size() - prevPtr) << "\033[0m";

    return out.str();
}

std::string decorate(
        std::string& in,
        TextColor tc = TextColor::tc_black,
        Style s = Style::s_none,
        BackgroundColor bc = BackgroundColor::bc_none) {
    return decorate(std::move(in), tc, s, bc);
}

template <typename T>
std::string boldRed(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_red, Style::s_bold);
}

template <typename T>
std::string boldGreen(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_green, Style::s_bold);
}

template <typename T>
std::string red(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_red);
}

template <typename T>
std::string green(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_green);
}

template <typename T>
std::string boldYellow(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_yellow, Style::s_bold);
}

template <typename T>
std::string yellow(T&& in) {
    return decorate(std::forward<T>(in), TextColor::tc_yellow);
}

} // namespace print
