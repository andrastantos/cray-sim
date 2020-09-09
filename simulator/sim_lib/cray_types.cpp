#include "utils.h"
#include "cray_types.h"

const size_t cSideEffectIdent = 20;

CAddr_t FromString(const std::string &aString, StringFormat_e aFormat) {
	if ((aFormat == StringFormat_e::ProgramAddr) || (aFormat == StringFormat_e::Both)) {
		CProgramAddrTranslator_c ProgramAddrTranslator;
		boost::optional<CProgramAddr_t> Val = ProgramAddrTranslator.get_value(aString);
		if (Val != boost::none) {
			return Val.get();
		}
	}
	if ((aFormat == StringFormat_e::DataAddr) || (aFormat == StringFormat_e::Both)) {
		CAddrTranslator_c AddrTranslator;
		boost::optional<CAddr_t> Val = AddrTranslator.get_value(aString);
		if (Val != boost::none) {
			return Val.get();
		}
	}
	throw StringFormatError_x();
}

boost::optional<std::string> CAddrTranslator_c::put_value(const CAddr_t& aValue) {
	std::stringstream StrStrm;
	StrStrm << HexPrinter(aValue,6);
	return boost::optional<std::string>(StrStrm.str());
}

boost::optional<std::string> CProgramAddrTranslator_c::put_value(const CProgramAddr_t& aValue) {
	std::stringstream StrStrm;
	StrStrm << HexPrinter(size_t(aValue)>>2,6) << ":p" << DecPrinter(size_t(aValue) & 3);
	return boost::optional<std::string>(StrStrm.str());
}



