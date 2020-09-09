#ifndef __PARSER_H__
#define __PARSER_H__

#include "utils.h"
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include <boost/optional.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////
// A very simple regex-based tokenizer
/////////////////////////////////////////////////////////////////////////////////////////////////

enum class Skip_e {
	No = 0,
	Yes
};

template <typename tIdType> class TokenDef_T {
public:
	TokenDef_T(const char *aFilter, const tIdType aTokenId, const Skip_e aSkip) : mTokenId(aTokenId), mSkip(aSkip) {
		std::string Filter = "^";
		Filter.append(aFilter);
		mFilter.assign(Filter);
	}
	virtual ~TokenDef_T() {}
	const char *Match(const char *aFirst, const char *aEnd) const {
		std::match_results<const char *> Result;
		bool RetVal = std::regex_search(aFirst, aEnd, Result, mFilter);
		if (RetVal) {
			return Result.suffix().first;
		}
		return nullptr;
	}
	bool Skip() const { return mSkip == Skip_e::Yes; }
protected:
	std::regex mFilter;
	tIdType mTokenId;
	Skip_e mSkip;

	template <typename tTokenIdType> friend class Token_t;
};

template <typename tIdType> class Token_t {
public:
	Token_t(const TokenDef_T<tIdType> &aTokenDef, const char *aFirst, const char *aLast) : mTokenId(aTokenDef.mTokenId), mValue(aFirst, aLast) {}
	explicit Token_t(const tIdType &aTokenId) : mTokenId(aTokenId) {}
	tIdType mTokenId;
	std::string mValue;
};

enum class TokenTypes_e {
	String,
	Comment,
	Delimiter,
	Word,
	EndOfLine,
	Whitespace,
	Any,
};

typedef Token_t<TokenTypes_e> Token_c;

typedef Token_c(*PostProcess_f)(const TokenDef_T<TokenTypes_e> &aTokenDef, const char *aFirst, const char *aLast);

class TokenDef_c : public TokenDef_T<TokenTypes_e> {
public:
	TokenDef_c(const char *aFilter, const TokenTypes_e aTokenId, const Skip_e aSkip, PostProcess_f aPostProcess = DefaultPostProcess) : TokenDef_T<TokenTypes_e>(aFilter, aTokenId, aSkip), mPostPorcess(aPostProcess) {}

	Token_c PostProcess(const char *aFirst, const char *aLast) const { return mPostPorcess(*this, aFirst, aLast); }
protected:
	static Token_c DefaultPostProcess(const TokenDef_T<TokenTypes_e> &aTokenDef, const char *aFirst, const char *aLast) {
		return Token_c(aTokenDef, aFirst, aLast);
	}
	PostProcess_f mPostPorcess;
};

typedef std::vector<Token_c> TokenStream_t;

TokenStream_t Tokenize(std::istream aStrm);
TokenStream_t Tokenize(std::string aStr);

#endif // __PARSER_H__
