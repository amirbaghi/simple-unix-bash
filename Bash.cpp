#include "Bash.h"
#include <regex>
#include <unistd.h>
#include <csignal>
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
        int status = waitpid(pid, NULL, 0);
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

pid_t Bash::find_pid(std::map<pid_t, std::string> bg_processes, int n)
{
    // Finding the pid based on the n given
    auto j = 1;
    for (auto i = bg_processes.begin(); i != bg_processes.end(); ++i, ++j) {
        return i->first;    
    }

    return -1;
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

void Bash::send_signal(pid_t pid, int signal_code)
{
    int status = kill(pid, signal_code);
    if (status == -1)
    {
        cout << "ERROR: Signal failed to be sent." << endl;
    }
}

void Bash::check_children(map<int, string>& current_bg_processes)
{
    int status;

        int bg_status = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED);

        // If a background process has changed states
        if (bg_status > 0)
        {
            // The process has started continuing
            if (WIFCONTINUED(status))
            {
                // Removing the "(Stopped) " from the beginning of the process name in the map
                string process_name = current_bg_processes[bg_status];
                current_bg_processes[bg_status] = current_bg_processes[bg_status].substr(10, process_name.size() - 1);
                cout << "Background process " << bg_status << " has continued executing." << endl;
            }
            // The process has been stopped
            else if (WIFSTOPPED(status))
            {
                // Adding "(Stopped) " to the beginning of the process name in the map
                string process_name = current_bg_processes[bg_status];
                current_bg_processes[bg_status] = "(Stopped) " + process_name;
                cout << "Background process " << bg_status << " has been stopped." << endl;
            }
            // The process has been terminated by a signal
            else if (WIFSIGNALED(status))
            {
                // Erase the background process from the map
                current_bg_processes.erase(bg_status);
                cout << "Background process " << bg_status << " has been terminated." << endl;
            }
            // The process has exitted normally
            else if (WIFEXITED(status))
            {
                // Erase the background process from the map
                current_bg_processes.erase(bg_status);
                cout << "Background process " << bg_status << " has finished executing." << endl;
            }
        }
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
        // Checking for background processes
        check_children(current_bg_processes);

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
        else if (args[0] == "bgkill")
        {
            pid_t pid = find_pid(current_bg_processes, stoi(args[1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else 
            {
                send_signal(pid, SIGTERM);
            }
        }
        else if (args[0] == "bgstop")
        {
            pid_t pid = find_pid(current_bg_processes, stoi(args[1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else
            {
                send_signal(pid, SIGSTOP);
            }
        }
        else if (args[0] == "bgstart")
        {
            pid_t pid = find_pid(current_bg_processes, stoi(args[1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else
            {
                send_signal(pid, SIGCONT);
            }
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
