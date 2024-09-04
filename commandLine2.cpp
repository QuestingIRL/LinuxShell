#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

using namespace std;

// Split a string by a delimiter into a vector of strings
vector<string> splitString(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Convert a vector of strings to a NULL-terminated array of char* for execvp
vector<char*> stringVecToCharArr(const vector<string>& vec) {
    vector<char*> arr;
    for (auto& str : vec) {
        arr.push_back(const_cast<char*>(str.c_str())); // Unsafe cast for simplicity
    }
    arr.push_back(nullptr); // execvp expects a NULL-terminated array
    return arr;
}

// Function to execute a single command, possibly with piping
void executeCommand(const string& cmd) {
    // Trim leading and trailing spaces
    auto trim = [](const string& str) {
        size_t first = str.find_first_not_of(' ');
        if (string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    };

    string trimmedCmd = trim(cmd);
    if (trimmedCmd.empty()) return; // Skip empty commands

    // Check if the command starts with 'cd'
    if (trimmedCmd.substr(0, 2) == "cd") {
        vector<string> parts = splitString(trimmedCmd, ' ');
        if (parts.size() < 2) {
            cerr << "cd: path required" << endl;
            return;
        }
        if (chdir(parts[1].c_str()) != 0) {
            perror("chdir failed");
        }
        return;
    }

    size_t pipePos = trimmedCmd.find('|');
    if (pipePos != string::npos) {
        // Handle piping logic here, as before
    } else {
        // Execute a single command without piping
        pid_t pid = fork();
        if (pid == 0) { // Child process
            vector<string> args = splitString(trimmedCmd, ' ');
            vector<char*> argv = stringVecToCharArr(args);
            execvp(argv[0], argv.data());
            // If execvp returns, it must have failed
            cerr << "Failed to execute command: " << trimmedCmd << endl;
            exit(EXIT_FAILURE);
        } else if (pid > 0) { // Parent process
            wait(nullptr); // Wait for the child process to finish
        } else {
            perror("fork failed");
        }
    }
}

// Shell main loop
void shellLoop() {
    string input;
    cout << "Welcome to the simple shell. Type 'exit' to quit." << endl;
    do {
        cout << "$> "; // Prompt
        getline(cin, input); // Take in the user's input
        if (input == "exit") break; // Exit command

        // If the user wants to run different commands, register this using ';'
        // Then plug the string into the splitString function
        vector<string> commands = splitString(input, ';');
        for (auto& cmd : commands) {
            executeCommand(cmd);
        }
    } while (true);
}

int main() {
    shellLoop(); // Start the shell
    return 0;
}
