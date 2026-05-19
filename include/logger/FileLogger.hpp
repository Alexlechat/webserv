#ifndef FileLogger_HPP
# define FileLogger_HPP

# include "logger/Logger.hpp"
# include <fstream>

class	FileLogger : public Logger
{
	public:
		explicit FileLogger(const std::string& path, bool truncate = true);
		virtual ~FileLogger(void);

		virtual void	log(const std::string& msg, LogLevel level, const char* file, int line);

		bool			isOpen(void) const;

	private:
		std::ofstream	_file;
		
		FileLogger(const FileLogger&);
		FileLogger& operator=(const FileLogger&);
};

#endif  // FileLogger_HPP
