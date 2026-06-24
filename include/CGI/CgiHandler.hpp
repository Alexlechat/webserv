#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include <string>
# include <vector>
# include <sys/types.h>
# include "../request/HttpRequest.hpp"
# include "../config/ServerConfig.hpp"
# include "../config/LocationConfig.hpp"

class CgiHandler
{
    public:
        CgiHandler(const HttpRequest& request, const LocationConfig& location);
        ~CgiHandler();

        int     execute();
        pid_t   getPid() const;
        int     getPipeFd() const;

    private:
        const HttpRequest&          _request;
        const LocationConfig&       _location;
        pid_t                       _pid;
        int                         _pipeFd;

        std::string                 _resolveInterpreter() const;
        std::string                 _resolveScriptPath() const;
        std::vector<std::string>    _buildEnv() const;
        void                        _setupChild(int pipeOut[2], int pipeIn[2]);
        void                        _setupParent(int pipeOut[2], int pipeIn[2]);
};

#endif