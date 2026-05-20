#include "utils/Utils.hpp"

std::string	Utils::strToLower(const std::string& s)
{
	std::string	out = s;
	for (size_t	i = 0; i < out.size(); ++i)
		out[i] = std::tolower((unsigned char)out[i]);

	return out;
}

std::string	Utils::trim(const std::string& s, const char* pattern)
{
	size_t	start = s.find_first_not_of(pattern);
	if (start == std::string::npos) return "";
	size_t	end = s.find_last_not_of(pattern);

	return s.substr(start, end - start + 1);
}
