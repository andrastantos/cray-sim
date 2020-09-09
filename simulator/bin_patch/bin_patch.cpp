#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include "utils.h"
#include "config_file.h"
#include <fstream>
#include <regex>
#include <string>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>

int PrintUsage(const char *aExecName, const char *ErrorStr = nullptr) {
	if (ErrorStr != nullptr) std::cout << "Error: " << ErrorStr << std::endl;
	std::cout << "Usage: " << aExecName << " <image file> <patch file> <output file> [<options>]" << std::endl;
	std::cout << "  The following options are available:" << std::endl;
	std::cout << "    -m: allow multiple matches" << std::endl;
	return 1;
}

std::regex CommentLine("[ \t]*#.*");
std::regex MatchLine("[ \t]*(0x[0-9a-fA-F]+):(0x[0-9a-fA-F]+)[ \t]*(#.*)?");
std::regex PatchLine("[ \t]*(0x[0-9a-fA-F]+):(0x[0-9a-fA-F]+)[ \t]*(0x[0-9a-fA-F]+):(0x[0-9a-fA-F]+)[ \t]*(#.*)?");

struct ValMask_s {
	uint16_t mValue;
	uint16_t mMask;
};

struct Patch_s {
	ValMask_s mMatch;
	ValMask_s mPatch;
};

std::string Trim(const std::string& aStr, const std::string& aWhitespace = " \t") {
	const auto FirstNonSpace = aStr.find_first_not_of(aWhitespace);
	if (FirstNonSpace == std::string::npos) return "";

	const auto LastNonSpace = aStr.find_last_not_of(aWhitespace);
	const auto TrimRange = LastNonSpace - FirstNonSpace + 1;

	return aStr.substr(FirstNonSpace, TrimRange);
}

boost::optional<Patch_s> ParseLine(std::string &aLine, size_t aLineNo) {
	std::string Line = Trim(aLine);
	if (Line.length() == 0) return boost::optional<Patch_s>(); // Empty lines are skipped
	std::smatch Matches;
	if (std::regex_match(Line, Matches, PatchLine)) {
		CRAY_ASSERT(Matches.size() > 5);
		Patch_s Patch;
		Patch.mMatch.mValue = FromString<uint16_t>(Matches[1]);
		Patch.mMatch.mMask = FromString<uint16_t>(Matches[2]);
		Patch.mPatch.mValue = FromString<uint16_t>(Matches[3]);
		Patch.mPatch.mMask = FromString<uint16_t>(Matches[4]);
		return Patch;
	}
	if (std::regex_match(Line, Matches, MatchLine)) {
		CRAY_ASSERT(Matches.size() > 3);
		Patch_s Patch;
		Patch.mMatch.mValue = FromString<uint16_t>(Matches[1]);
		Patch.mMatch.mMask = FromString<uint16_t>(Matches[2]);
		Patch.mPatch.mValue = 0;
		Patch.mPatch.mMask = 0x0000; // Patch is not applied
		return Patch;
	}
	if (std::regex_match(Line, CommentLine)) return boost::optional<Patch_s>(); // Comment lines are skipped
	throw Generic_x() << "Can't parse line " << DecPrinter(aLineNo);
}

class PatchFile_c {
public:
	PatchFile_c() {}
	void Read(std::ifstream &aStrm) {
		size_t LineNo = 1;
		while (!aStrm.eof()) {
			std::string Line;
			std::getline(aStrm, Line);
			boost::optional<Patch_s> Patch = ParseLine(Line, LineNo);
			if (Patch.is_initialized()) {
				mPatch.push_back(Patch.get());
			}
		}
	}
	std::vector<Patch_s> mPatch;

	unsigned int Apply(std::vector<uint16_t> &aArray) {
		unsigned int MatchCount = 0;
		size_t Start = 0;
		size_t LastStart = aArray.size() - mPatch.size();
		while (Start < LastStart) {
			bool Found = true;
			for (size_t Idx = 0; Idx < mPatch.size(); ++Idx) {
				if (((aArray[Start + Idx] ^ mPatch[Idx].mMatch.mValue) & mPatch[Idx].mMatch.mMask) != 0) {
					Found = false;
					break;
				}
			}
			if (Found) {
				for (size_t Idx = 0; Idx < mPatch.size(); ++Idx) {
					aArray[Start + Idx] =
						(aArray[Start + Idx] & ~mPatch[Idx].mPatch.mMask) |
						(mPatch[Idx].mPatch.mValue & mPatch[Idx].mPatch.mMask);
				}
				++MatchCount;
			}
			++Start;
		}
		return MatchCount;
	}
};




int main(int argc, const char* argv[])
{
	CommandLine_c CommandLine(argc, argv);
	std::string InFileName;
	std::string PatchFileName;
	std::string OutFileName;
	bool UniqueMatch = true;

	try {
		while (CommandLine.HasMoreParams()) {
			std::string CurParam = CommandLine.GetNextParam();
			if (CurParam.length() == 0) continue;
			if (CurParam == "-m") {
				UniqueMatch = false;
			}
			else if (InFileName.empty()) {
				InFileName = CurParam;
			}
			else if (PatchFileName.empty()) {
				PatchFileName = CurParam;
			}
			else if (OutFileName.empty()) {
				OutFileName = CurParam;
			}
			else {
				throw Generic_x("Unkown command line parameter");
			}
		}
		if (InFileName.length() == 0) throw Generic_x() << "Input file must be specified";
		if (!boost::filesystem::exists(InFileName)) throw Generic_x() << "File: " << InFileName << " doesn't exist";
		if (PatchFileName.length() == 0) throw Generic_x() << "Patch file must be specified";
		if (!boost::filesystem::exists(PatchFileName)) throw Generic_x() << "File: " << PatchFileName << " doesn't exist";
		if (OutFileName.length() == 0) throw Generic_x() << "Output file must be specified";
		
		size_t FileSize = boost::filesystem::file_size(InFileName);
		if ((FileSize % 2) != 0) throw Generic_x() << "This prorgam only supports binary files with even number of bytes";
		std::vector<uint16_t> FileContent(FileSize / 2);
		std::ifstream InFile(InFileName, std::ios::binary | std::ios::in);
		InFile.read((char*)&(FileContent[0]), FileSize);
		if (InFile.bad()) throw Generic_x() << "Can't read input file: " << InFileName;
		InFile.close();
		for (auto &Word : FileContent) {
			Word = SwapBytes(Word);
		}

		std::ifstream PatchStrm(PatchFileName, std::ios::in);
		if (PatchStrm.bad()) throw Generic_x() << "Can't open patch file: " << PatchFileName;

		PatchFile_c PatchFile;
		PatchFile.Read(PatchStrm);
		PatchStrm.close();
	
		unsigned int MatchCnt = PatchFile.Apply(FileContent);

		if (MatchCnt == 0) {
			std::cout << "Patch wasn't applied: no match was found" << std::endl;
			return 2;
		}
		if (MatchCnt > 1 && UniqueMatch) {
			std::cout << "Patch wasn't applied: multiple matches were found" << std::endl;
			return 3;
		}
		std::cout << "Patch applied " << MatchCnt << " times" << std::endl;

		std::ofstream OutFile(OutFileName, std::ios::binary | std::ios::out);
		if (OutFile.bad()) throw Generic_x() << "Can't open output file " << OutFileName;
		for (auto &Word : FileContent) {
			Word = SwapBytes(Word);
		}
		OutFile.write((char*)&(FileContent[0]), FileSize);
		if (OutFile.bad()) throw Generic_x() << "Can't write output file: " << OutFileName;
		OutFile.close();

		return 0;
	}
	catch(std::exception &Ex) {
		return PrintUsage(argv[0], Ex.what());
	}
}

