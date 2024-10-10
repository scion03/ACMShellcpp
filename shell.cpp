#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h> // for chdir(), fork(), exec(), pid_t
#include <cstdlib>  // for exit, execvp, EXIT_SUCCESS, EXIT_FAILURE
#include <cstdio>
#include <cstring> // for strcmp
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// Constants
constexpr int ACMShell_TOK_BUFSIZE = 64;
const char *ACMShell_TOK_DELIM = " \t\r\n\a";

// Function Declarations
int ACMShell_cd(char **args);
int ACMShell_help(char **args);
int ACMShell_exit(char **args);
int ACMShell_history(char **args);

// Typedef for builtin functions
typedef int (*builtin_func_ptr)(char **);

// Builtin command names
const std::vector<std::string> builtin_str = {
    "cd",
    "help",
    "exit",
    "history"};

// Corresponding builtin functions
const std::vector<builtin_func_ptr> builtin_func = {
    ACMShell_cd,
    ACMShell_help,
    ACMShell_exit,
    ACMShell_history};

// Function to get the number of builtins
int ACMShell_num_builtins()
{
  return static_cast<int>(builtin_str.size());
}

// Implementing cd builtin
int ACMShell_cd(char **args)
{
  if (args[1] == nullptr)
  {
    std::cerr << "ACMShell: expected argument to \"cd\"\n";
  }
  else
  {
    if (chdir(args[1]) != 0)
    {
      perror("ACMShell");
    }
  }
  return 1;
}

// Implementing help builtin
int ACMShell_help(char **args)
{
  std::cout << "ACM's very own shell\n";
  std::cout << "Type program names and arguments, and hit enter.\n";
  std::cout << "The following are built in:\n";

  for (size_t i = 0; i < builtin_str.size(); ++i)
  {
    std::cout << "  " << builtin_str[i] << "\n";
  }

  return 1;
}

// History using a vector of strings
std::vector<std::string> history;

// Function to add commands to history
void add_to_hist(char **args)
{
  std::string cmd;
  for (int i = 0; args[i] != nullptr; ++i)
  {
    if (i > 0)
      cmd += " ";
    cmd += args[i];
  }
  history.push_back(cmd);
}

// Implementing history builtin
int ACMShell_history(char **args)
{
  int i = 1;
  for (const auto &cmd : history)
  {
    std::cout << " " << i++ << " " << cmd << "\n";
  }
  return 1;
}

// Implementing exit builtin
int ACMShell_exit(char **args)
{
  return 0;
}



// Function to launch non-builtin commands
int ACMShell_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    // Child process
    if (execvp(args[0], args) == -1)
    {
      perror("ACMShell");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    // Error forking
    perror("ACMShell");
  }
  else
  {
    // Parent process waits for child to finish
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// Function to execute commands
int ACMShell_execute(char **args)
{
  if (args[0] == nullptr)
  {
    // Empty command was entered
    return 1;
  }

  // Check if the command is a builtin
  for (size_t i = 0; i < builtin_str.size(); ++i)
  {
    if (strcmp(args[0], builtin_str[i].c_str()) == 0)
    {
      return (*builtin_func[i])(args);
    }
  }

  // If not a builtin, launch it
  return ACMShell_launch(args);
}

// Function to read a line of input
std::string ACMShell_read_line()
{
  std::string line;
  if (!std::getline(std::cin, line))
  {
    // If EOF is encountered, exit the shell
    exit(EXIT_SUCCESS);
  }
  return line;
}

// Function to split a line into tokens
std::vector<char *> ACMShell_split_line(const std::string &line)
{
  std::vector<char *> tokens;
  std::istringstream iss(line);
  std::string token;
  while (iss >> token)
  {
    tokens.push_back(strdup(token.c_str()));
  }
  tokens.push_back(nullptr); // Null-terminate the array
  return tokens;
}

// Main loop of the shell
void ACMShell_loop()
{
  std::string line;
  std::vector<char *> args;
  int status;

  do
  {
    std::cout << "> ";
    line = ACMShell_read_line();
    args = ACMShell_split_line(line);
    add_to_hist(args.data());
    status = ACMShell_execute(args.data());

    // Free the duplicated strings
    for (auto ptr : args)
    {
      if (ptr != nullptr)
        free(ptr);
    }
  } while (status);
}

int main(int argc, char **argv)
{
  // Run the main loop
  ACMShell_loop();
  return EXIT_SUCCESS;
}