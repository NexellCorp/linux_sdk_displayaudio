#ifndef NXUTILS_H
#define NXUTILS_H

#include <vector>
#include <string>
using namespace std;

#define COMMAND_FORMAT_STX			'$'
#define COMMAND_FORMAT_REPLY_DONE	"OK"
#define COMMAND_FORMAT_REPLY_FAIL	"NG"
#define COMMAND_FORMAT_SEPERATOR	'#'
#define COMMAND_FORMAT_ETX			'\n'

enum CommandType {
	CommandType_Service,
	CommandType_Command,
	CommandType_Parameter
};

vector<string> CreateTokensFromCommand(const char* command);

vector<string> CreateTokens(string text, char seperator, char stx/*= 0*/, char etx/*= 0*/);

string FindArgument(string* command);

bool FindArgument(string* command, string target, string* argument);

string MakeReplyCommand(bool ok, vector<string> reply);

bool IsDigit(std::string text);

string ToLower(string str);

string ToUpper(string str);

#endif /* NXUTILS_H */
