#include "NxUtils.h"
#include <locale>
#include <string.h>

#define BUFFER_SIZE 1024
#define DEVICE_ADDRESS_SIZE 6

string ToStringBTMacId(unsigned char* bd_addr, int len, char seperator)
{
	char buffer[BUFFER_SIZE] = {0,};

	if (len < DEVICE_ADDRESS_SIZE) {
		return std::string();
	}

	sprintf(buffer, "%02x%c%02x%c%02x%c%02x%c%02x%c%02x", bd_addr[0] & 0xFF, seperator, bd_addr[1] & 0xFF, seperator, bd_addr[2] & 0xFF, seperator, bd_addr[3] & 0xFF, seperator, bd_addr[4] & 0xFF, seperator, bd_addr[5] & 0xFF);

	return (std::string)buffer;
}


vector<string> CreateTokensFromCommand(const char* command)
{
	string target = (string)command;
	vector<string> tokens;
	string token;
	int stx, etx;

	stx = target.find("$");
	if (stx < 0) {
		goto error_occur;
	}

	etx = target.find("\n", stx);
	if (etx < 0) {
		goto error_occur;
	}

	for (int i = stx+1; i < etx; i++) {
		if (target[i] == '#') {
			tokens.push_back(token);
			token.clear();
			continue;
		}

		token += target[i];
	}

	if (token.length()) {
		tokens.push_back(token);
	}

error_occur:
	return tokens;
}

vector<string> CreateTokens(string text, char seperator, char stx/*= 0*/, char etx/*= 0*/)
{
	vector<string> tokens;
	string token;
	int start = 0, end = text.length();

	if (stx) {
		start = text.find(stx);

		if (start < 0) {
			goto loop_finish;
		}
	}

	if (etx) {
		end = text.find(etx, stx);

		if (end < 0) {
			goto loop_finish;
		}
	}

	for (int i = start; i < end; i++) {
		if (text[i] == seperator) {
			tokens.push_back(token);
			token.clear();
			continue;
		}

		token += text[i];
	}

	if (token.length()) {
		tokens.push_back(token);
	}

loop_finish:
	return tokens;
}

string FindArgument(std::string* command)
{
	size_t pos = command->rfind(" ");
	if (pos == string::npos) {
		return string();
	}
	++pos;

	return command->substr(pos, command->length()-pos);
}

bool FindArgument(string* command, string target, string* argument)
{
	argument->clear();

	for (size_t i = strlen(target.c_str()); i < command->length(); i++) {
		*argument += command->at(i);
	}

	return !argument->empty();
}

string MakeReplyCommand(bool ok, vector<string> reply)
{
	std::string command;

	command += COMMAND_FORMAT_STX;
	command += ok ? COMMAND_FORMAT_REPLY_DONE : COMMAND_FORMAT_REPLY_FAIL;

	for (size_t i = 0; i < reply.size(); i++) {
		command += COMMAND_FORMAT_SEPERATOR + reply[i];
	}

	command += COMMAND_FORMAT_ETX;

	return command;
}

bool IsDigit(std::string text)
{
	if (text.empty()) {
		return false;
	}

	for (size_t i = 0; i < text.length(); i++) {
		if (text[i] < '0' || '9' < text[i]) {
			return false;
		}
	}

	return true;
}

string ToLower(string str)
{
	locale l;
	string ret;

	for (size_t i = 0; i < str.length(); ++i)
	{
		ret += tolower(str[i], l);
	}

	return ret;
}

string ToUpper(string str)
{
	locale l;
	string ret;

	for (size_t i = 0; i < str.length(); ++i)
	{
		ret += toupper(str[i], l);
	}

	return ret;
}
