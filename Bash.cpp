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
    regex delims("[^ \t\n]+");
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
    argv.reserve(args.size());

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
    // Executing
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
    // Waiting on the child process
    else
    {
        int status = waitpid(pid, NULL, WUNTRACED);
    }
}

void Bash::bg_exec(std::vector<std::string> &args, int *number_of_bg_processes)
{
    // Checking if we already have 5 processes running in the background
    if (*number_of_bg_processes == 0)
    {
        cout << "ERROR: Too many background processes already running." << endl;
        return;
    }

    // Incrementing the number of background processes
    *number_of_bg_processes += 1;

    // Making a vector of char pointers from the vector of strings
    std::vector<char *> argv;
    argv.reserve(args.size());

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
    // Executing
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
    else 
    {
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

void Bash::bash()
{
    string input;
    vector<string> args;
    int number_of_bg_processes = 0;
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
            number_of_bg_processes--;
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
            bg_exec(args, &number_of_bg_processes);
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
