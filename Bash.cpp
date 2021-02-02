#include "Bash.h"
#include <regex>
#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

void Bash::extract_commands(string &input, vector<vector<string>> &cmds)
{

    string current_command;
    // Clearing the cmds vector from previous cmds
    cmds.clear();

    // Regex for splitting the commands by |
    regex pipe("(?=[^|])(?:[^|]*\\([^)]+\\))*[^|]*");
    auto begin = sregex_iterator(input.begin(), input.end(), pipe);
    auto end = sregex_iterator();

    // Regex for single commands
    regex delims("(\"([^\"]+)\")|(\'([^\']+)\')|\\S+");
    sregex_iterator tokens_begin;
    sregex_iterator tokens_end;

    for (sregex_iterator i = begin; i != end; i++)
    {
        // A single command
        current_command = (*i).str();

        tokens_begin = sregex_iterator(current_command.begin(), current_command.end(), delims);
        tokens_end = sregex_iterator();

        std::vector<string> args;

        args.reserve(MAXIMUM_SIZE);

        // Filling the args vector with the tokens extracted for the current command
        for (sregex_iterator i = tokens_begin; i != tokens_end; i++)
        {
            // If the arg is double-quoted, add the arg string without the double-quotes
            if ((*i).str()[0] == '"')
            {
                args.push_back((*i).str(2));
            }
            // If the arg is single-quoted, add the arg string without the single-quotes
            else if ((*i).str()[0] == '\'')
            {
                args.push_back((*i).str(4));
            }
            else
            {
                args.push_back((*i).str());
            }
        }

        // Push the command in the cmds vector
        cmds.push_back(args);
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

void Bash::fg_single_exec(vector<std::string> &args, int in_stream, int out_stream)
{

    string filename;
    bool redir_in = false, redir_out = false;

    // Checking if there is a redirection operator (>) in the command
    auto iter = args.begin();
    iter = std::find_if(iter, args.end(), [](string s) -> bool { return s == ">"; });

    if (iter != args.end())
    {
        redir_out = true;
        filename = *(iter + 1);
        args.erase(iter, args.end());
    }

    // Checking if there is a redirection operator (<) in the command
    iter = args.begin();
    iter = std::find_if(iter, args.end(), [](string s) -> bool { return s == "<"; });

    if (iter != args.end())
    {
        redir_in = true;
        filename = *(iter + 1);
        args.erase(iter, args.end());
    }

    // Making a vector of char pointers from the args
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

        // Setting the file as STDIN if there is a "<" operator in the args (Redirection)
        if (redir_in)
        {
            int fd0;
            if ((fd0 = open(const_cast<char *>(filename.c_str()), O_RDONLY)) < 0)
            {
                cout << "ERROR: Couldn't open file " << filename << endl;
                exit(1);
            }
            dup2(fd0, STDIN_FILENO);
            close(fd0);
        }
        // Setting the given in_stream as STDIN if it's not 0 (Used for piping)
        else if (in_stream != STDIN_FILENO)
        {
            dup2(in_stream, STDIN_FILENO);
            close(in_stream);
        }

        // Setting the file as STDOUT if there is a ">" operator in the args (Redirection)
        if (redir_out)
        {
            int fd1;
            if ((fd1 = creat(const_cast<char *>(filename.c_str()), 0600)) < 0)
            {
                cout << "ERROR: Couldn't open/create file " << filename << endl;
                exit(1);
            }
            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }
        // Setting the given out_stream as STDOUT if it's not 1 (Used for piping)
        else if (out_stream != STDOUT_FILENO)
        {
            dup2(out_stream, STDOUT_FILENO);
            close(out_stream);
        }

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

void Bash::bg_exec(std::vector<std::string> &args, map<pid_t, string> &current_bg_processes)
{
    // Checking if we already have 5 processes running in the background
    if (current_bg_processes.size() == 5)
    {
        cout << "ERROR: Too many background processes already running." << endl;
        return;
    }

    string filename;
    bool redir_in = false, redir_out = false;

    // Checking if there is a redirection operator (>) in the command
    auto iter = args.begin();
    iter = std::find_if(iter, args.end(), [](string s) -> bool { return s == ">"; });

    if (iter != args.end())
    {
        redir_out = true;
        filename = *(iter + 1);
        args.erase(iter, args.end());
    }

    // Checking if there is a redirection operator (<) in the command
    iter = args.begin();
    iter = std::find_if(iter, args.end(), [](string s) -> bool { return s == "<"; });

    if (iter != args.end())
    {
        redir_in = true;
        filename = *(iter + 1);
        args.erase(iter, args.end());
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

        // Setting the file as STDIN if there is a "<" operator in the args
        if (redir_in)
        {
            int fd0;
            if ((fd0 = open(const_cast<char *>(filename.c_str()), O_RDONLY)) < 0)
            {
                cout << "ERROR: Couldn't open file " << filename << endl;
                exit(1);
            }
            dup2(fd0, STDIN_FILENO);
            close(fd0);
        }

        // Setting the file as STDOUT if there is a ">" operator in the args
        if (redir_out)
        {
            int fd1;
            if ((fd1 = creat(const_cast<char *>(filename.c_str()), 0600)) < 0)
            {
                cout << "ERROR: Couldn't open/create file " << filename << endl;
                exit(1);
            }
            dup2(fd1, STDOUT_FILENO);
            close(fd1);
        }

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
    for (auto i = bg_processes.begin(); i != bg_processes.end(); ++i, ++j)
    {
        return i->first;
    }

    return -1;
}

void Bash::bg_list(map<pid_t, std::string> bg_processes)
{
    // Printing the current background processes
    auto j = 1;
    for (auto i = bg_processes.begin(); i != bg_processes.end(); ++i, ++j)
    {
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

void Bash::check_children(map<int, string> &current_bg_processes)
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
    vector<vector<string>> cmds;
    map<pid_t, string> current_bg_processes;
    cmds.reserve(MAXIMUM_SIZE);

    // Main Bash Loop
    while (true)
    {
        // Checking for background processes
        check_children(current_bg_processes);

        // Printing the prompt and getting input from user
        cout << "shell> ";

        getline(cin, input);

        // Extracting the commands from the input
        extract_commands(input, cmds);

        // If nothing (whitespace, enter) was entered
        if (cmds.size() == 0)
        {
            continue;
        }
        // Exit command
        else if (cmds[0][0] == "exit")
        {
            exit(0);
        }
        // cd command
        else if (cmds[0][0] == "cd")
        {
            change_directory(cmds[0][1]);
        }
        // pwd command
        else if (cmds[0][0] == "pwd")
        {
            current_directory();
        }
        // Background execution (Piping is not supported in background execution)
        else if (cmds[0][0] == "bg")
        {
            bg_exec(cmds[0], current_bg_processes);
        }
        // Bglist, listing current background processes
        else if (cmds[0][0] == "bglist")
        {
            bg_list(current_bg_processes);
        }
        // Bgkill, signaling TERM to background process
        else if (cmds[0][0] == "bgkill")
        {
            // Finding the process based on the number given by the user
            pid_t pid = find_pid(current_bg_processes, stoi(cmds[0][1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else
            {
                send_signal(pid, SIGTERM);
            }
        }
        // Bgstop, signaling STOP to background process
        else if (cmds[0][0] == "bgstop")
        {
            // Finding the process based on the number given by the user
            pid_t pid = find_pid(current_bg_processes, stoi(cmds[0][1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else
            {
                send_signal(pid, SIGSTOP);
            }
        }
        // Bgstart, signaling START to background process
        else if (cmds[0][0] == "bgstart")
        {
            // Finding the process based on the number given by the user
            pid_t pid = find_pid(current_bg_processes, stoi(cmds[0][1]));
            if (pid == -1)
            {
                cout << "ERROR: No such process found" << endl;
            }
            else
            {
                send_signal(pid, SIGCONT);
            }
        }
        // Foreground execution (Piped or not)
        else
        {
            fg_exec(cmds);
        }
    }
}

void Bash::fg_exec(std::vector<std::vector<std::string>> &cmds)
{
    int current_in_stream;
    int pipe_fd[2];

    // Set the current input stream as STDIN, for the first process
    current_in_stream = STDIN_FILENO;

    // Iterate over commands and pipe them
    int i;
    for (i = 0; i < cmds.size() - 1; i++)
    {
        // Creating a new pipe
        pipe(pipe_fd);

        // Executing the current command in a child process, using the current input stream and the write end of the pipe
        fg_single_exec(cmds[i], current_in_stream, pipe_fd[1]);

        // Closing the write end of the pipe, because the parent doesn't need it
        close(pipe_fd[1]);

        // Setting the next input stream as the read end of the previous pipe
        current_in_stream = pipe_fd[0];
    }

    // Executing the last command using the current input stream and STDOUT as the output stream
    fg_single_exec(cmds[i], current_in_stream, STDOUT_FILENO);
    
}
