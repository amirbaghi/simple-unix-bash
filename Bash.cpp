#include "Bash.h"
#include <regex>
#include <sys/types.h>

using namespace std;

void Bash::extract_arguments(string &input, vector<std::string> &args)
{
    string temp;

    regex delims("[^ \t\n]+");
    auto tokens_begin = sregex_iterator(input.begin(), input.end(), delims);
    auto tokens_end = sregex_iterator();

    for (sregex_iterator i = tokens_begin ; i != tokens_end ; i++)
    {
        args.push_back((*i).str());
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
        cout << endl;

    }    
}


