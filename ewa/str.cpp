#include "str.hpp"

#include "log.hpp"

using std::string;

std::vector<std::string> SplitString(const std::string& str,
				     const std::string& delimiter, const bool keepDelimiters)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
	string token = str.substr(prev, pos - prev);



	if(token != delimiter && token.size() > 0) {
	    strings.push_back(token);
	}

	if(keepDelimiters) {
	    strings.push_back(delimiter);
	}

        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

bool BeginsWith(const std::string& str,
		const std::string& substr) {
    return substr == str.substr(0, substr.length());
}

void ToLowercase(std::string& str)  {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}
