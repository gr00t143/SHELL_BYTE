#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define LSH_TOK_DELIM " \t\r\n\a"

std::string lsh_read_line()
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::vector<std::string> lsh_split_line(const std::string& line)
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;
    
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    
    return tokens;
}

int lsh_launch(const std::vector<std::string>& args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        std::vector<char*> c_args;
        for (const auto& arg : args)
        {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);
        
        if (execvp(c_args[0], c_args.data()) == -1)
        {
            perror("lsh");
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0)
    {
        perror("lsh");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int lsh_cd(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        std::cerr << "lsh: expecting argument for \"cdir\"\n";
    }
    else
    {
        if (chdir(args[1].c_str()) != 0)
        {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(const std::vector<std::string>& args)
{
    std::cout << "\t\t\t\t cdir\n";
    std::cout << "\t\t\t\t help\n";
    std::cout << "\t\t\t\t ext\n";
    std::cout << "\t\t\t\t cwd\n";
    std::cout << "\t\t\t\t list\n";
    return 1;
}

int lsh_exit(const std::vector<std::string>& args)
{
    return 0;
}

int lsh_pwd(const std::vector<std::string>& args)
{
    char* cwd = getcwd(nullptr, 0);
    if (cwd == nullptr)
    {
        return 0;
    }

    std::cout << cwd << std::endl;

    free(cwd);

    return 1;
}

int lsh_list(const std::vector<std::string>& args)
{
    DIR* dir;
    struct dirent* entry;
    if (args.size() < 2)
    {
        dir = opendir(".");
    }
    else
    {
        dir = opendir(args[1].c_str());
    }
    if (dir == nullptr)
    {
        return 0;
    }
    while ((entry = readdir(dir)) != nullptr)
    {
        std::cout << entry->d_name << std::endl;
    }

    closedir(dir);

    return 1;
}

int lsh_execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        return 1;
    }

    if (args[0] == "cdir")
    {
        return lsh_cd(args);
    }
    else if (args[0] == "help")
    {
        return lsh_help(args);
    }
    else if (args[0] == "ext")
    {
        return lsh_exit(args);
    }
    else if (args[0] == "cwd")
    {
        return lsh_pwd(args);
    }
    else if (args[0] == "list")
    {
        return lsh_list(args);
    }
    else
    {
        return lsh_launch(args);
    }
}

void lsh_loop()
{
    std::string line;
    std::vector<std::string> args;
    int status;

    do
    {
        std::cout << "imGroot:> ";
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

    } while (status);
}

int main(int argc, char** argv)
{
    lsh_loop();
    return EXIT_SUCCESS;
}
