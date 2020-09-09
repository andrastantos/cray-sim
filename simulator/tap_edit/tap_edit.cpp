#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include <iostream>
#include <regex>
#include <string>
#include <fstream>
#include "tap_file.h"
#include <boost/filesystem.hpp>

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

Token_c PostProcessString(const TokenDef_T<TokenTypes_e> &aTokenDef, const char *aFirst, const char *aLast) {
	++aFirst;
	--aLast;
	Token_c RetVal(aTokenDef, aFirst, aLast);
	// Replace all "" sequences with a single "
	RetVal.mValue = std::regex_replace(RetVal.mValue, std::regex("\"\""), "\"");
	return RetVal;
}

TokenDef_c TokenDefs[] {
	{ "[\r\t ]+", TokenTypes_e::Whitespace, Skip_e::Yes },
	{ "\"([^\"]|\"\")*\"", TokenTypes_e::String, Skip_e::No, PostProcessString },
	{ "#.*$", TokenTypes_e::Comment, Skip_e::No },
	{ "[;=]", TokenTypes_e::Delimiter, Skip_e::No },
	{ "[^ \r\t\n;=]+", TokenTypes_e::Word, Skip_e::No },
	{ ".", TokenTypes_e::Any, Skip_e::No }
};

const char *TokenStr(TokenTypes_e aToken) {
	switch (aToken) {
	case TokenTypes_e::String: return "string";
	case TokenTypes_e::Comment: return "comment";
	case TokenTypes_e::Delimiter: return "delimiter";
	case TokenTypes_e::Word: return "word";
	case TokenTypes_e::EndOfLine: return "EOL";
	case TokenTypes_e::Whitespace: return "Whitespace";
	case TokenTypes_e::Any: return "any";
	default:
		throw Generic_x() << "Unkown token type: " << size_t(aToken);
	}
	return nullptr;
}

std::string FormatErrorLine(const std::string& aLine, const char *TokenPtr, size_t aLineNo) {
	size_t Pos = TokenPtr - aLine.c_str();
	std::stringstream Strm;
	Strm << "Syntax error at line: " << aLineNo << std::endl;
	Strm << aLine << std::endl;
	for (size_t i = 0; i < Pos; ++Pos) { Strm << ' '; }
	Strm << '^';
	return Strm.str();
}

std::string FormatErrorLine(const std::string& aLine, const char *TokenPtr) {
	size_t Pos = TokenPtr - aLine.c_str();
	std::stringstream Strm;
	Strm << "Syntax error in command: " << std::endl;
	Strm << aLine << std::endl;
	for (size_t i = 0; i < Pos; ++Pos) { Strm << ' '; }
	Strm << '^';
	return Strm.str();
}

typedef std::vector<Token_c> TokenStream_t;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////////////////////////////

static const int FileIdxEnd = -1;

void SeekToFile(TapFile_c &aFile, int aFileIdx) {
	if (aFileIdx != FileIdxEnd && aFileIdx < 0) throw Generic_x() << "Invalid file index: " << DecPrinter(aFileIdx);

	aFile.SeekToBeginningOfTape();
	int FileNo = 0;
	while ((aFileIdx == FileIdxEnd) || (aFileIdx != FileNo)) {
		if (aFile.GetState() == TapFile_c::State_e::EndOfTape) throw Generic_x() << "File with index " << DecPrinter(aFileIdx) << " doesn't exist";
		do {
			std::vector<uint8_t> Buffer;
			Buffer = aFile.Read(4096); // 4k records are the most common, so let's use that as the baseline
		} while (aFile.GetState() != TapFile_c::State_e::EndOfFile && aFile.GetState() != TapFile_c::State_e::EndOfTape);
		++FileNo;
	}
}

int ParsePos(const std::string &aStrPos, bool aAllowSpecialValues = true) {
	if (aAllowSpecialValues && aStrPos == "end") return FileIdxEnd;
	try {
		int Pos = std::stoi(aStrPos, nullptr, 10);
		if (Pos < 0) throw Generic_x("Invalid position specified");
		return Pos;
	}
	catch (std::out_of_range &) {
		throw Generic_x("Invalid position specified");
	}
}

void Add(std::ifstream &aSrc, TapFile_c &aTarget, size_t aRecordSize = 4096) {
	std::vector<uint8_t> Buffer(aRecordSize);
	while (!aSrc.eof()) {
		aSrc.read((char*)(&Buffer[0]), aRecordSize);
		size_t ReadCount = size_t(aSrc.gcount());
		if (ReadCount != aRecordSize && !aSrc.eof()) throw Generic_x() << "Can't read from input file";
		if (ReadCount > 0) {
			Buffer.resize(ReadCount);
			aTarget.WriteRecord(Buffer);
		}
	}
	aTarget.WriteEndOfFile();
	aTarget.flush();
}

void Add(TapFile_c &aSrc, TapFile_c &aTarget) {
	std::vector<uint8_t> Buffer;
	bool NeedEOF = false;
	bool First = true;
	do {
		Buffer = aSrc.Read();
		if (aSrc.GetState() == TapFile_c::State_e::EndOfTape) {
			if (First) return;
			throw Generic_x() << "Unexpected end of input file";
		}
		First = false;
		if (aSrc.GetState() == TapFile_c::State_e::EndOfFile) break;
		aTarget.WriteRecord(Buffer);
		NeedEOF = true;
	} while (true);
	if (NeedEOF) aTarget.WriteEndOfFile();
	aTarget.flush();
}

void Add(TapFile_c &aSrc, std::ofstream &aTarget) {
	std::vector<uint8_t> Buffer;
	bool First = true;
	do {
		Buffer = aSrc.Read();
		if (aSrc.GetState() == TapFile_c::State_e::EndOfTape) {
			if (First) return;
			throw Generic_x() << "Unexpected end of input file";
		}
		First = false;
		if (aSrc.GetState() == TapFile_c::State_e::EndOfFile) break;
		aTarget.write((char*)(&Buffer[0]), Buffer.size());
		if (!aTarget.good()) throw Generic_x() << "Can't write to file";
	} while (true);
	aTarget.flush();
}

void Skip(TapFile_c &aSrc) {
	std::vector<uint8_t> Buffer;
	bool First = true;
	do {
		aSrc.SeekToNextRecord();
		if (aSrc.GetState() == TapFile_c::State_e::EndOfTape) {
			if (First) return;
			throw Generic_x() << "Unexpected end of input file";
		}
		First = false;
		if (aSrc.GetState() == TapFile_c::State_e::EndOfFile) break;
	} while (true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Commands
/////////////////////////////////////////////////////////////////////////////////////////////////

class CmdFactoryBase_i {
public:
	virtual ~CmdFactoryBase_i() {}
	virtual std::unique_ptr<class CmdBase_i> Parse(TokenStream_t::const_iterator &aBegin, TokenStream_t::const_iterator aEnd) = 0;
	virtual std::string Help() const = 0;
};

class CmdBase_i {
public:
	virtual ~CmdBase_i() {}
	virtual void Exec(TapFile_c &aTarget) = 0;
};

class CmdLs_c : public CmdFactoryBase_i {
public:
	virtual ~CmdLs_c() {}
	virtual std::unique_ptr<class CmdBase_i> Parse(TokenStream_t::const_iterator &aBegin, TokenStream_t::const_iterator aEnd) override {
		if (aBegin->mValue != "ls") return nullptr;
		++aBegin;
		if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return nullptr;
		return std::make_unique<Exec_c>();
	}
	virtual std::string Help() const override {
		return "ls                        - Lists the content of the tape file";
	}
	class Exec_c : public CmdBase_i {
		virtual void Exec(TapFile_c &aTarget) override {
			std::cout << "TAP FILE: " << aTarget.GetFileName() << std::endl;
			if (!aTarget.good()) {
				std::cout << "    File empty" << std::endl;
				return;
			}
			aTarget.SeekToBeginningOfTape();
			size_t FileNo = 0;
			while (aTarget.GetState() != TapFile_c::State_e::EndOfTape) {
				size_t FileSize = 0;
				size_t RecCnt = 1;
				do {
					std::vector<uint8_t> Buffer;
					Buffer = aTarget.Read();
					if (aTarget.GetState() == TapFile_c::State_e::EndOfTape) break;
					FileSize += Buffer.size();
					if (aTarget.GetState() == TapFile_c::State_e::EndOfRecord) ++RecCnt;
				} while (aTarget.GetState() != TapFile_c::State_e::EndOfFile && aTarget.GetState() != TapFile_c::State_e::EndOfTape);
				if (aTarget.GetState() != TapFile_c::State_e::EndOfTape) {
					std::cout << "    File: " << DecPrinter(FileNo) << " size: " << DecPrinter(FileSize) << " record count: " << DecPrinter(RecCnt) << std::endl;
				}
				++FileNo;
			}
		}
	};
};

// - extract <position> <file name>
class CmdExtract_c : public CmdFactoryBase_i {
public:
	virtual ~CmdExtract_c() override {}
	virtual std::unique_ptr<class CmdBase_i> Parse(TokenStream_t::const_iterator &aBegin, TokenStream_t::const_iterator aEnd) override {
		// format of the command:
		//    extract <position> <file name>
		if (aBegin->mValue != "extract") return nullptr;
		++aBegin;
		if (aBegin == aEnd) return nullptr;
		int Pos = ParsePos(aBegin->mValue, false);
		++aBegin;
		if (aBegin == aEnd) return nullptr;
		std::string FileName = aBegin->mValue;
		++aBegin;
		if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return nullptr;

		// TODO: add warning about existing file
		return std::make_unique<Exec_c>(FileName, Pos);
	}
	virtual std::string Help() const override {
		return "extract <pos> <file name> - Extracts the file at (0-based) position into file.";
	}
	class Exec_c : public CmdBase_i {
	public:
		Exec_c(std::string aFileName, int aPos) : mFileName(aFileName), mPos(aPos) {}
		virtual void Exec(TapFile_c &aTarget) override {
			SeekToFile(aTarget, mPos);
			std::vector<uint8_t> Buffer(mRecordSize);
			std::ofstream File(mFileName, std::ios_base::out | std::ios_base::binary);
			if (!File.good()) throw Generic_x() << "Can't open file: " << mFileName;
			Add(aTarget, File);
		}
		std::string mFileName;
		int mPos;
		size_t mRecordSize;
	};
};

// - replace <position> <file name> (uses temp files)
// - delete <position> (uses temp files)
// - insert <file name> [<position>] (uses temp files)
class CmdReplaceInsertDelete_c : public CmdFactoryBase_i {
public:
	virtual ~CmdReplaceInsertDelete_c() override {}
	virtual std::unique_ptr<class CmdBase_i> Parse(TokenStream_t::const_iterator &aBegin, TokenStream_t::const_iterator aEnd) override {
		// format of the command:
		//    extract <position> <file name>
		if (aBegin->mValue == "replace") {
			++aBegin;
			if (aBegin == aEnd) return nullptr;
			int Pos = ParsePos(aBegin->mValue);
			++aBegin;
			if (aBegin == aEnd) return nullptr;
			std::string FileName = aBegin->mValue;
			++aBegin;
			if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return nullptr;
			return std::make_unique<Exec_c>(FileName, Pos, Mode_e::Replace);
		} else if (aBegin->mValue == "delete") {
			++aBegin;
			if (aBegin == aEnd) return nullptr;
			int Pos = ParsePos(aBegin->mValue, false);
			++aBegin;
			if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return nullptr;
			return std::make_unique<Exec_c>(Pos, Mode_e::Delete);
		} else if (aBegin->mValue == "insert") {
			++aBegin;
			if (aBegin == aEnd) return nullptr;
			std::string FileName = aBegin->mValue;
			++aBegin;
			if (aBegin == aEnd) return nullptr;
			int Pos = ParsePos(aBegin->mValue);
			++aBegin;
			if (aBegin != aEnd && aBegin->mTokenId != TokenTypes_e::EndOfLine) return nullptr;
			return std::make_unique<Exec_c>(FileName, Pos, Mode_e::Insert);
		} else {
			return nullptr;
		}
	}
	virtual std::string Help() const override {
		return "insert <file name> <pos>  - Inserts the file at (0-based) position. \n"
			   "                            Position can be specified as 'end' in which case file gets inserted at the end of the tape\n"
		       "replace <pos> <file name> - Replaces file at (0-based) position.\n"
		       "delete <pos>              - Delete file at (0-based) position.\n"
		       "                            Position can be specified as 'end' in which case file gets inserted at the end of the tape";
	}
	enum class Mode_e {
		Replace = 0,
		Delete,
		Insert
	};
	class Exec_c : public CmdBase_i {
	public:
		Exec_c(std::string aFileName, int aPos, Mode_e aMode) : mFileName(aFileName), mPos(aPos), mMode(aMode) {}
		Exec_c(int aPos, Mode_e aMode) : mPos(aPos), mMode(aMode) {}
		virtual void Exec(TapFile_c &aTarget) override {
			aTarget.SeekToBeginningOfTape();
			boost::filesystem::path TempFile = boost::filesystem::unique_path();
			std::ifstream File;
			if (mMode != Mode_e::Delete) {
				File.open(mFileName, std::ios_base::in | std::ios_base::binary);
				if (!File.good()) throw Generic_x() << "Can't open file: " << mFileName;
			}
			TapFile_c TempTape(TempFile.string());
			for (int i = 0; i < mPos || mPos == FileIdxEnd; ++i) {
				Add(aTarget, TempTape);
				if (aTarget.GetState() == TapFile_c::State_e::EndOfTape) break;
			}
			if (mMode == Mode_e::Insert || mMode == Mode_e::Replace) {
				Add(File, TempTape);
			}
			if (aTarget.GetState() != TapFile_c::State_e::EndOfTape) {
				if (mMode == Mode_e::Delete || mMode == Mode_e::Replace) {
					Skip(aTarget);
				}
			}
			while (aTarget.GetState() != TapFile_c::State_e::EndOfTape) {
				Add(aTarget, TempTape);
			}
			TempTape.close();
			File.close();
			aTarget.close();
			boost::filesystem::rename(TempFile, aTarget.GetFileName());
			aTarget.ReOpen();
		}
		std::string mFileName;
		int mPos;
		size_t mRecordSize;
		Mode_e mMode;
	};
};

const std::unique_ptr<CmdFactoryBase_i> Commands[] = {
	std::make_unique<CmdLs_c>(),
	std::make_unique<CmdExtract_c>(),
	std::make_unique<CmdReplaceInsertDelete_c>()
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// Main
/////////////////////////////////////////////////////////////////////////////////////////////////

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	std::cout << "Usage: " << aExecName << " <tape file name> [-s <script file>] [command]" << std::endl;
	std::cout << "Available commands are: " << std::endl;
	for (const auto &Command : Commands) {
		std::cout << Command->Help() << std::endl;
	}
	std::cout << "Each command is put on a new line in a script file" << std::endl;
	std::cout << "Multiple commands on the command line can be separated by '--'" << std::endl;
	return 1;
}

int main(int argc, const char* argv[]) {
	CommandLine_c CommandLine(argc, argv);
	std::string TapFileName;
	std::string ScriptFileName;
	std::string Command;
	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			{
				if (TapFileName.empty()) {
					TapFileName = CurParam;
				}
				else if (CurParam == "-s") {
					if (!ScriptFileName.empty()) throw Generic_x("Script file is already specified");
					ScriptFileName = CommandLine.GetNextParam();
				}
				else {
					Command.append(" ");
					Command.append(CurParam);
				}
			}
		}
		if (TapFileName.empty()) throw Generic_x("Tape file must be specified");
		if (Command.empty() && ScriptFileName.empty()) throw Generic_x("Either a script file or a command must be specified");
		if (!Command.empty() && !ScriptFileName.empty()) throw Generic_x("Script file and commmand cannot be specified at the same time");
	}
	catch (std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}

	try {
		TokenStream_t Tokens;
		if (!ScriptFileName.empty()) {
			std::ifstream ScriptFile(ScriptFileName);
			if (!ScriptFile.is_open()) throw Generic_x() << "Couldn't open file: " << ScriptFileName;
			ScriptFile.unsetf(std::ios::skipws);      // No white space skipping!
			size_t LineNo = 1;

			// Tokenize the input file
			while (!ScriptFile.eof() && ScriptFile.good()) {
				std::string Line;
				getline(ScriptFile, Line);
				if (!ScriptFile.good()) break;
				const char *First = Line.c_str();
				const char *Last = First + Line.size();
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
						throw Generic_x() << FormatErrorLine(Line, First, LineNo);
					}
				}
				Tokens.push_back(Token_c(TokenTypes_e::EndOfLine));
				++LineNo;
			}
			if (!ScriptFile.eof()) {
				throw Generic_x() << "Error reading file: " << ScriptFileName;
			}
		}
		else if (!Command.empty()) {
			const char *First = Command.c_str();
			const char *Last = First + Command.size();
			while (First != Last) {
				bool Found = false;
				for (const auto &TokenDef : TokenDefs) {
					const char *NewFirst = TokenDef.Match(First, Last);
					if (NewFirst != nullptr) {
						Found = true;
						if (!TokenDef.Skip()) {
							Token_c Token = TokenDef.PostProcess(First, NewFirst);
							if (Token.mValue == "--") {
								Tokens.push_back(Token_c(TokenTypes_e::EndOfLine));
							}
							else {
								Tokens.push_back(Token);
							}
						}
						First = NewFirst;
						break;
					}
				}
				if (!Found) {
					throw Generic_x() << FormatErrorLine(Command, First);
				}
			}
			Tokens.push_back(Token_c(TokenTypes_e::EndOfLine));
		}
		else {
			CRAY_ASSERT(false); // We should not get here: we've checked before that exactly one of these two strings have content
		}

		std::vector<std::unique_ptr<CmdBase_i>> AST;
		// Create an AST - which for us is a simple list of command objects
		TokenStream_t::const_iterator Token = Tokens.cbegin();
		TokenStream_t::const_iterator TokenEnd = Tokens.cend();
		while (Token != TokenEnd) {
			std::unique_ptr<CmdBase_i> Node;
#ifndef NO_ASSERTS
			TokenStream_t::const_iterator LastValid = Token;
#endif
			while (Token != TokenEnd && Token->mTokenId == TokenTypes_e::EndOfLine) ++Token;
			if (Token == TokenEnd) break;
			for (const auto &Command : Commands) {
				Node = Command->Parse(Token, TokenEnd);
				if (Node != nullptr) break;
			}
			if (Node == nullptr) {
				// None of the commands managed to parse the current token
				throw Generic_x() << "Can't interpret script file after token: " << Token->mValue;
			}
			CRAY_ASSERT(LastValid != Token);
			AST.emplace_back(move(Node));
		}

		// Open the TAP file
		TapFile_c TapFile;
		TapFile.open(TapFileName);
		//if (!TapFile.good()) throw Generic_x() << "Can't open TAP file: " << TapFileName;

		// Execute the AST
		for (auto &Node : AST) {
			Node->Exec(TapFile);
			TapFile.flush();
		}

		return 0;
	}
	catch (Generic_x &E) {
		std::cout << "ERROR: " << E.what() << std::endl;
		return 1;
	}
	catch (std::exception &E) {
		std::cout << "Program terminated with exception: " << E.what() << std::endl;
		return 1;
	}
}


