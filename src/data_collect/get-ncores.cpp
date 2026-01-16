#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

int getval(std::string space, std::string key) {
    std::string val = space.substr(space.find(key));
    val = val.substr(key.length(), val.find("\n")-key.length());
    ltrim(val);
    return std::stoi(val);
}

int get_ncores() {
    std::string lscpu = exec("lscpu");
    size_t found = lscpu.find("Socket(s):");
    std::string remainder = lscpu.substr(found);
    remainder = remainder.substr(0, remainder.find("\n"));

    return getval(lscpu, "Core(s) per socket:")*getval(lscpu, "Socket(s):"); 
} // TODO: add flag for using hyperthreading