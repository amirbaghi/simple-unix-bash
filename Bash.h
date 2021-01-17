#include <iostream>
#include <vector>
#ifndef BASH_MINE_H
#define BASH_MINE_H

namespace Bash
{

    // Main function responsible for getting input and calling appropriate functions
    void bash();

    // Function responsible for extracting arguments from input into a vector of strings
    std::vector<std::string> extract_arguments(std::string input);

    // Function responsible for executing foreground programs
    void fg_exec(std::vector<std::string> args);

}; // namespace Bash

#endif