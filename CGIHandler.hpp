#pragma once

#include "Utility.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <vector>

class CGIHandler {
    public:
        static std::string execute(const std::string& scriptPath, const Client& client) {
        
            int pipe_in[2];  // Parent -> Child (Write body)
            int pipe_out[2]; // Child -> Parent (Read HTML)

            if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1){
                return "Error: Pipe failed";
            } 

            pid_t pid = fork();
            if (pid == -1) {
                return "Error: Fork failed";
            }

            if (pid == 0) { // child process
            
            // read from the STDIN
            dup2(pipe_in[0], STDIN_FILENO);
            // wite to STDOUT
            dup2(pipe_out[1], STDOUT_FILENO);
            
            close(pipe_in[0]); 
            close(pipe_in[1]);
            close(pipe_out[0]);
            close(pipe_out[1]);

            std::vector<std::string> env;
            env.push_back("REQUEST_METHOD=" + (client.request.http_method == METHOD::POST ? std::string("POST") : std::string("GET")));
            env.push_back("CONTENT_LENGTH=" + std::to_string(client.request.content_length));
            env.push_back("QUERY_STRING=" + client.request.query_string);
            env.push_back("CONTENT_TYPE=" + client.request.content_type);
            env.push_back("PATH_INFO=" + client.request.path);
            env.push_back("SCRIPT_FILENAME=" + scriptPath);

            std::vector<char*> envp;
            for (auto& s : env) envp.push_back(const_cast<char*>(s.c_str()));
            envp.push_back(nullptr);

            char* argv[] = { const_cast<char*>(scriptPath.c_str()), nullptr };

            // execute the script 
            execve(scriptPath.c_str(), argv, envp.data());
            exit(1);

        } else { // parent process
            close(pipe_in[0]);
            
            // write body to the child process
            if (client.request.http_method == METHOD::POST && !client.request.body.empty()) {
                write(pipe_in[1], client.request.body.c_str(), client.request.body.size());
            }

            close(pipe_in[1]);
            close(pipe_out[1]);

            // read output
            std::string result;
            char buffer[4096];
            ssize_t bytesRead;
            while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0) {
                result.append(buffer, bytesRead);
            }
            close(pipe_out[0]);
            
            // zombie cleanup 
            waitpid(pid, nullptr, 0); 
            return (result);
        }
    }
};