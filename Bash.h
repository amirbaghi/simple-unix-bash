#include <iostream>
#include <vector>
#ifndef BASH_MINE_H
#define BASH_MINE_H
#define MAXIMUM_SIZE 1000

namespace Bash
{

    // Main function responsible for getting input and calling appropriate functions
    void bash();

    // Function responsible for extracting arguments from input into a vector of strings
    void extract_arguments(std::string &input, std::vector<std::string> &args);

    // Function responsible for executing foreground programs
    void fg_exec(std::vector<std::string> &args);

    // Function responsible for executing background programs
    void bg_exec(std::vector<std::string> &args, int* number_of_bg_processes);

    // Function responsible for changing directory
    void change_directory(std::string directory);

    // Function responsible for printing current directory
    void current_directory();

}; // namespace Bash

#endif