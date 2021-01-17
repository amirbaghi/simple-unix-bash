#include <iostream>
#include <map>
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
    void bg_exec(std::vector<std::string> &args, std::map<pid_t, std::string>& current_bg_processes);

    // Function responsible for changing directory
    void change_directory(std::string directory);

    // Function responsible for printing current directory
    void current_directory();

    // Function responsible for listing the background processes
    void bg_list(std::map<pid_t, std::string> bg_processes);

    // Function for sending signal
    void send_signal(pid_t pid, int signal_code);

    // Function to find pid based on an n given
    pid_t find_pid(std::map<pid_t, std::string> bg_processes, int n);

    // Function to wait on child processes and checking and updating accordingly
    void check_children(std::map<int, std::string>& current_bg_processes);

}; // namespace Bash

#endif