#include "utils/Utils.hpp"

#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <vector>
#include <stdexcept>

std::string	Utils::strToLower(const std::string& s)
{
	std::string	out = s;
	for (size_t	i = 0; i < out.size(); ++i)
		out[i] = std::tolower((unsigned char)out[i]);

	return out;
}

std::string	Utils::strToUpper(const std::string& s)
{
	std::string	out = s;
	for (size_t	i = 0; i < out.size(); ++i)
		out[i] = std::toupper((unsigned char)out[i]);

	return out;
}

std::string	Utils::trim(const std::string& s, const char* pattern)
{
	size_t	start = s.find_first_not_of(pattern);
	if (start == std::string::npos) return "";
	size_t	end = s.find_last_not_of(pattern);

	return s.substr(start, end - start + 1);
}

std::string	Utils::readFile(const std::string& filepath)
{
	std::ifstream	file(filepath.c_str());

	if (!file)
		throw	std::runtime_error("Failed to open file: " + filepath);

	std::stringstream	ss;

	ss << file.rdbuf();
	return ss.str();
}

std::string	Utils::normalizePath(const std::string& path)
{
	std::vector<std::string>	stack;
	std::istringstream			iss(path);
	std::string					segment;

	while (std::getline(iss, segment, '/'))
	{
		if (segment.empty() || segment == ".")
			continue;
		if (segment == "..")
		{
			// Trying to climb above the root: reject entirely.
			if (stack.empty())
				return "";
			stack.pop_back();
			continue;
		}
		stack.push_back(segment);
	}

	std::string	out = "/";
	for (size_t i = 0; i < stack.size(); ++i)
	{
		out += stack[i];
		if (i + 1 < stack.size())
			out += "/";
	}
	return out;
}

std::string	Utils::sanitizeFilename(const std::string& filename)
{
	// Keep only the final path component (strip any directories),
	// then reject anything empty or made only of dots.
	size_t	slash = filename.find_last_of("/\\");
	std::string	base = (slash == std::string::npos) ? filename : filename.substr(slash + 1);

	base = trim(base, " \t\r\n");

	bool	onlyDots = !base.empty();
	for (size_t i = 0; i < base.size(); ++i)
	{
		if (base[i] != '.')
		{
			onlyDots = false;
			break;
		}
	}

	if (base.empty() || onlyDots)
		return "";

	return base;
}

std::string	Utils::ipv4ToString(unsigned int hostOrderAddr)
{
	std::ostringstream	oss;

	oss << ((hostOrderAddr >> 24) & 0xFF) << '.'
		<< ((hostOrderAddr >> 16) & 0xFF) << '.'
		<< ((hostOrderAddr >> 8) & 0xFF) << '.'
		<< (hostOrderAddr & 0xFF);

	return oss.str();
}

bool		Utils::dir_exists(const std::string& path)
{
	struct stat	info;

	if (stat(path.c_str(), &info) != 0)
		return false;

	return (info.st_mode & S_IFDIR) != 0;
}

bool		Utils::file_exists(const std::string& filepath)
{
	struct stat	info;

	if (stat(filepath.c_str(), &info) != 0)
		return false;

	return (info.st_mode & S_IFREG) != 0;
}
