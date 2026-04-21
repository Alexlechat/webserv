#ifndef ConfigParser_HPP
# define ConfigParser_HPP

# include <string>
# include <vector>
# include <exception>

class	ConfigParser
{
	public:
		ConfigParser(const std::string& configFilePath);

		class	ParseException : public std::exception
		{
			public:
				ParseException(const std::string& msg) : _msg(msg) {}
				virtual const char* what(void) const throw() { return _msg.c_str(); }
				virtual ~ParseException() throw() {}

			private:
				std::string	_msg;
		};

	private:
		std::vector<std::string>	_tokens;
		size_t						_pos;

		void		_tokenize(const std::string& fileContent);
		std::string	_getFileContent(const std::string& configFilePath);
};

#endif  // ConfigParser_HPP
