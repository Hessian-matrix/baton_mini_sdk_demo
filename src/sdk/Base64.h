#ifndef __BASE64__
#define __BASE64__

#include <string>
#include <cstring>

#include <iostream>
using namespace std;

class CBase64
{
public:
	CBase64();
	virtual ~CBase64();

	std::string encode(char const* bytes_to_encode, unsigned int in_len);
	std::string decode(std::string const& encoded_string);
};

#endif