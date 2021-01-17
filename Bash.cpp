#include "Bash.h"
#include <regex>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void Bash::extract_arguments(string &input, vector<string> &args)
{
    string temp;
    args.clear();

    regex delims("[^ \t\n]+");
    auto tokens_begin = sregex_iterator(input.begin(), input.end(), delims);
    auto tokens_end = sregex_iterator();

    for (sregex_iterator i = tokens_begin; i != tokens_end; i++)
    {
        args.push_back((*i).str());
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


    int status;
    pid_t pid = fork();

    if (pid < 0)
    {
        cout << "ERROR: Forking Process Failed" << endl;
        exit(1);
    }
    else if (pid == 0)
    {
        status = execvp(argv[0], &argv[0]);
        if (status < 0)
        {
            cout << "ERROR: Executing Failed" << endl;
            exit(1);
        }
    }
    else
    {
        while (wait(NULL) != pid)
            ;
    }
}

void Bash::bash()
{
    string input;
    vector<string> args;
    args.reserve(MAXIMUM_SIZE);

    while (true)
    {
        cout << "shell> ";
        getline(cin, input);
        extract_arguments(input, args);

        if (args[0] == "exit")
        {
            exit(0);
        }
        fg_exec(args);

        cout << endl;
    }
}

int main(int argc, char **argv)
{
    Bash::bash();
    return 0;
}
