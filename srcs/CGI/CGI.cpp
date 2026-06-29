#include "CGI/CgiHandler.hpp"
#include "request/Response.hpp"

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

//CONSTRUCTOR
CgiHandler::CgiHandler(const HttpRequest& request, const LocationConfig& location) :
    _request(request),
    _location(location),
    _pid(-1),
    _pipeFd(-1) {}

CgiHandler::~CgiHandler()
{
    if (_pipeFd >= 0)
    {
        close(_pipeFd);
    }
}


//GETTER
pid_t   CgiHandler::getPid() const
{
    return _pid;
}

int CgiHandler::getPipeFd() const
{
    return _pipeFd;
}


//EXECUTION
std::string CgiHandler::execute()
{
    std::string interpreter = _resolveInterpreter();
    if (interpreter.empty())
    {
        return buildErrorResponse(500, ServerConfig());
    }

    int pipeOut[2];
    int pipeIn[2];

    if (pipe(pipeOut) < 0 || pipe(pipeIn) < 0)
    {
        return buildErrorResponse(500, ServerConfig());
    }

    _pid = fork();
    if (_pid < 0)
    {
        return buildErrorResponse(500, ServerConfig());
    }
    if (_pid == 0)
    {
        _setupChild(pipeOut, pipeIn);
    }
    else
    {
        _setupParent(pipeOut, pipeIn);
    }

    std::string cgiOutput;
    char        buf[4096];
    ssize_t     n;

    while ((n = read(_pipeFd, buf, sizeof(buf))) > 0)
    {
        cgiOutput.append(buf, n);
    }

    waitpid(_pid, NULL, 0);

    return "HTTP/1.1 200 OK\r\nConnection: close\r\n" + cgiOutput;
}


//CHILD
void    CgiHandler::_setupChild(int pipeOut[2], int pipeIn[2])
{
    close(pipeOut[0]);
    dup2(pipeOut[1], STDOUT_FILENO);
    close(pipeOut[1]);

    close(pipeIn[1]);
    dup2(pipeIn[0], STDIN_FILENO);
    close(pipeIn[0]);

    //build env
    std::vector<std::string>    envVec = _buildEnv();
    std::vector<char*>          env;
    for (size_t i = 0; i < envVec.size(); i++)
    {
        env.push_back(const_cast<char*>(envVec[i].c_str()));
    }
    env.push_back(NULL);

    //build args[]
    std::string interpreter = _resolveInterpreter();
    std::string scriptPath = _resolveScriptPath();
    char* args[] = {
        const_cast<char*>(interpreter.c_str()),
        const_cast<char*>(scriptPath.c_str()),
        NULL
    };

    chdir(_location.root.c_str());
    execve(interpreter.c_str(), args, env.data());

    std::cerr << "execve() failed\n";
    exit(1);
}


//PARENT
void    CgiHandler::_setupParent(int pipeOut[2], int pipeIn[2])
{
    close(pipeOut[1]);
    close(pipeIn[0]);

    if (_request.method == "POST" && !_request.body.empty())
    {
        write(pipeIn[1], _request.body.c_str(), _request.body.size());
    }

    close(pipeIn[1]);
    _pipeFd = pipeOut[0];
}