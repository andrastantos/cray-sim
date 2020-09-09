#include "parser.h"

static Token_c PostProcessString(const TokenDef_T<TokenTypes_e> &aTokenDef, const char *aFirst, const char *aLast) {
	++aFirst;
	--aLast;
	Token_c RetVal(aTokenDef, aFirst, aLast);
	// Replace all "" sequences with a single "
	RetVal.mValue = std::regex_replace(RetVal.mValue, std::regex("\"\""), "\"");
	return RetVal;
}

static TokenDef_c TokenDefs[]{
	{ "[\r\t ]+", TokenTypes_e::Whitespace, Skip_e::Yes },
	{ "\"([^\"]|\"\")*\"", TokenTypes_e::String, Skip_e::No, PostProcessString },
	{ "#.*$", TokenTypes_e::Comment, Skip_e::No },
	{ "[;=]", TokenTypes_e::Delimiter, Skip_e::No },
	{ "[^ \r\t\n;=]+", TokenTypes_e::Word, Skip_e::No },
	{ ".", TokenTypes_e::Any, Skip_e::No }
};

//const char *TokenStr(TokenTypes_e aToken) {
//	switch (aToken) {
//	case TokenTypes_e::String: return "string";
//	case TokenTypes_e::Comment: return "comment";
//	case TokenTypes_e::Delimiter: return "delimiter";
//	case TokenTypes_e::Word: return "word";
//	case TokenTypes_e::EndOfLine: return "EOL";
//	case TokenTypes_e::Whitespace: return "Whitespace";
//	case TokenTypes_e::Any: return "any";
//	default:
//		throw Generic_x() << "Unkown token type: " << size_t(aToken);
//	}
//	return nullptr;
//}

template <typename tAdditionalInfo> static std::string FormatErrorLine(const std::string& aLine, const char *TokenPtr, std::string aAdditionalInfoHeader = "", boost::optional<tAdditionalInfo> aAdditionalInfo = boost::optional<tAdditionalInfo>()) {
	size_t Pos = TokenPtr - aLine.c_str();
	std::stringstream Strm;
	if (aAdditionalInfo.is_initialized()) {
		Strm << aAdditionalInfoHeader << aAdditionalInfo << std::endl;
	}
	Strm << aLine << std::endl;
	for (size_t i = 0; i < Pos; ++Pos) { Strm << ' '; }
	Strm << '^';
	return Strm.str();
}

template <typename tAdditionalInfo> static TokenStream_t Tokenize(std::string aStr, const std::string &aAdditionalInfoHeader, boost::optional<tAdditionalInfo> aAdditionalInfo = boost::optional<tAdditionalInfo>()) {
	return Tokenize(aStr, aAdditionalInfo.c_str(), aAdditionalInfo);
}

template <typename tAdditionalInfo> static TokenStream_t Tokenize(std::string aStr, const char *aAdditionalInfoHeader = "", boost::optional<tAdditionalInfo> aAdditionalInfo = boost::optional<tAdditionalInfo>()) {
	TokenStream_t Tokens;

	const char *First = aStr.c_str();
	const char *Last = First + aStr.size();
	while (First != Last) {
		bool Found = false;
		for (const auto &TokenDef : TokenDefs) {
			const char *NewFirst = TokenDef.Match(First, Last);
			if (NewFirst != nullptr) {
				Found = true;
				if (!TokenDef.Skip()) Tokens.push_back(TokenDef.PostProcess(First, NewFirst));
				First = NewFirst;
				break;
			}
		}
		if (!Found) {
			throw Generic_x() << FormatErrorLine(aStr, First, aAdditionalInfoHeader, aAdditionalInfo);
		}
	}
	Tokens.push_back(Token_c(TokenTypes_e::EndOfLine));
	return Tokens;
}

TokenStream_t Tokenize(std::string aStr) {
	return Tokenize<size_t>(aStr); // Let's use the same template instance that's already there for the file version...
}

TokenStream_t Tokenize(std::istream aStrm) {
	aStrm.unsetf(std::ios::skipws);      // No white space skipping!
	TokenStream_t Tokens;
	size_t LineNo = 1;

	// Tokenize the input file
	while (!aStrm.eof() && aStrm.good()) {
		std::string Line;
		getline(aStrm, Line);
		if (!aStrm.good()) break;
		TokenStream_t LineTokens = Tokenize<size_t>(Line, "Syntax error at line: ", LineNo);
		Tokens.insert(Tokens.end(), LineTokens.begin(), LineTokens.end());
		++LineNo;
	}
	if (!aStrm.eof()) {
		throw Generic_x() << "Can't tokenize stream";
	}
	return Tokens;
}
