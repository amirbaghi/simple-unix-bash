#include "Bash.h"
#include <regex>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void Bash::extract_arguments(string &input, vector<string> &args)
{
    string temp;
    // Clearing the args vector from previous args
    args.clear();

    // Applying Regex
    regex delims("[^\\s\"]+|([\"\'])(?:(?=(\\?))\2.)*?\1");
    auto tokens_begin = sregex_iterator(input.begin(), input.end(), delims);
    auto tokens_end = sregex_iterator();
    
    // Filling the args vector with the tokens
    for (sregex_iterator i = tokens_begin; i != tokens_end; i++)
    {
        args.push_back((*i).str());
    }
}

void Bash::change_directory(std::string directory)
{
    // Change Directory
    int status = chdir(const_cast<char *>(directory.c_str()));

    // Check for errors
    if (status != 0)
    {
        cout << "ERROR: Changing directory failed" << endl;
        exit(1);
    }
}

void Bash::fg_exec(vector<std::string> &args)
{

    // Making a vector of char pointers from the vector of strings
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);

    for (int i = 0; i < args.size(); ++i)
        argv.push_back(const_cast<char *>(args[i].c_str()));

    argv.push_back(NULL);

    // Forking a child process
    pid_t pid = fork();

    // Checking for error
    if (pid < 0)
    {
        cout << "ERROR: Forking Process Failed" << endl;
        exit(1);
    }
    // Executing (in child process)
    else if (pid == 0)
    {
        int status = execvp(argv[0], &argv[0]);
        // Checking for error
        if (status < 0)
        {
            cout << "ERROR: Executing Failed" << endl;
            exit(1);
        }
    }
    // Waiting on the child process (in parent process)
    else
    {
        int status = waitpid(pid, NULL, WUNTRACED);
    }
}

void Bash::bg_exec(std::vector<std::string> &args, map<pid_t, string>& current_bg_processes)
{
    // Checking if we already have 5 processes running in the background
    if (current_bg_processes.size() == 5)
    {
        cout << "ERROR: Too many background processes already running." << endl;
        return;
    }

    // Making a vector of char pointers from the vector of strings
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);

    for (int i = 0; i < args.size(); ++i)
        argv.push_back(const_cast<char *>(args[i].c_str()));

    argv.push_back(NULL);

    // Forking a child process
    pid_t pid = fork();

    // Checking for error
    if (pid < 0)
    {
        cout << "ERROR: Forking Process Failed" << endl;
        exit(1);
    }
    // Executing (in child process)
    else if (pid == 0)
    {
        int status = execvp(argv[1], &argv[1]);
        // Checking for error
        if (status < 0)
        {
            cout << "ERROR: Executing Failed" << endl;
            exit(1);
        }
    }
    // Printing a message indicating the execution of the background process (in the parent process)
    else 
    {
        // Including the new background process into the map of background processes
        current_bg_processes[pid] = args[1];

        cout << "Background process " << pid << " started executing." << endl;
    }
}

void Bash::current_directory()
{
    char s[100];

    // Getting the current working directory and putting it into s
    getcwd(s, 100);

    // Printing
    cout << s << endl;
}

void Bash::bg_list(map<pid_t, std::string> bg_processes)
{
    // Printing the current background processes
    auto j = 1;
    for (auto i = bg_processes.begin(); i != bg_processes.end(); ++i, ++j) {
        cout << "(" << j << ") " << i->second << endl;    
    }

    cout << "Total background jobs: " << bg_processes.size() << endl;
}


void Bash::bash()
{
    string input;
    vector<string> args;
    map<pid_t, string> current_bg_processes;
    args.reserve(MAXIMUM_SIZE);

    // Main Bash Loop
    while (true)
    {
        // Checking for any finished background processes

        int bg_status = waitpid(-1, NULL, WNOHANG);

        // If a background process is finished
        if (bg_status > 0)
        {
            // Print message and decrement the number of background processes
            cout << "Background process " << bg_status << " has finished executing." << endl;
            // Erase the background process from the map
            current_bg_processes.erase(bg_status);
        }

        // Printing the prompt and getting input from user
        cout << "shell> ";
        getline(cin, input);

        // Extracting the arguments from the input
        extract_arguments(input, args);

        // Running the appropriate function based on the first argument
        if (args[0] == "exit")
        {
            exit(0);
        }
        else if (args[0] == "cd")
        {
            change_directory(args[1]);
        }
        else if (args[0] == "pwd")
        {
            current_directory();
        }
        else if (args[0] == "bg")
        {
            bg_exec(args, current_bg_processes);
        }
        else if (args[0] == "bglist")
        {
            bg_list(current_bg_processes);
        }
        else
        {
            fg_exec(args);
        }
    }
}

int main(int argc, char **argv)
{
    Bash::bash();
    return 0;
}
