#ifndef __CONFIG_FILE_H__
#define __CONFIG_FILE_H__

#include <stdint.h>
#include <string>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/format.hpp>
#include "cray_types.h"
#include "exceptions.h"
#include "utils.h"

/////////////////////////////////////////////////////////////////////
// A translator class for custom types used in the configuration file

// Custom translator for bool (only supports std::string)
class BoolTranslator_c {
public:
	typedef std::string internal_type;
	typedef bool        external_type;

	boost::optional<bool> get_value(const std::string &aStr) {
		if (aStr.empty()) return boost::optional<bool>(boost::none);
		return boost::optional<bool>((boost::algorithm::iequals(aStr, "true") || boost::algorithm::iequals(aStr, "yes") || aStr == "0"));
	}
	boost::optional<std::string> put_value(const bool& aValue) {
		return boost::optional<std::string>(aValue ? "true" : "false");
	}
};

// This works for all integer types, except for int64_t
template <typename tIntType> class IntTranslator_c {
public:
	typedef std::string internal_type;
	typedef tIntType    external_type;

	boost::optional<tIntType> get_value(const std::string &aStr) {
		try {
			if (aStr.empty()) return boost::optional<tIntType>(boost::none);
			size_t EndIdx;
			uint64_t Data = std::stoull(aStr, &EndIdx, 0);
			if (EndIdx != aStr.length()) return boost::optional<tIntType>(boost::none);
			// Range check trick: convert to target type then back and see if we get the same value.
			tIntType RetVal = tIntType(Data);
			if (Data != uint64_t(RetVal)) return boost::optional<tIntType>(boost::none);
			return boost::optional<tIntType>(RetVal);
		}
		catch (std::exception &) {
			return boost::optional<tIntType>(boost::none);
		}
	}
	boost::optional<std::string> put_value(const tIntType &aValue) {
		std::stringstream StrStrm;
		StrStrm << aValue;
		return boost::optional<std::string>(StrStrm.str());
	}
};

// specialization for int64_t
template <> class IntTranslator_c<int64_t> {
public:
	typedef std::string internal_type;
	typedef int64_t     external_type;

	boost::optional<int64_t> get_value(const std::string &aStr) {
		try {
			if (aStr.empty()) return boost::optional<int64_t>(boost::none);
			size_t EndIdx;
			int64_t RetVal = std::stoll(aStr, &EndIdx, 0);
			if (EndIdx != aStr.length()) return boost::optional<int64_t>(boost::none);
			return boost::optional<int64_t>(RetVal);
		}
		catch (std::exception &) {
			return boost::optional<int64_t>(boost::none);
		}
	}
	boost::optional<std::string> put_value(const int64_t &aValue) {
		std::stringstream StrStrm;
		StrStrm << aValue;
		return boost::optional<std::string>(StrStrm.str());
	}
};

template <typename tReturnType> tReturnType FromString(const std::string &aString) {
	typename boost::property_tree::translator_between<std::string, tReturnType>::type Translator;
	return Translator.get_value(aString).get();
}

#ifdef FROM_STRING
#error FROM_STRING already defined
#endif
#define FROM_STRING(aRetVal)                                                         \
	template <> inline  aRetVal FromString<aRetVal>(const std::string &aStr) {       \
		boost::optional<aRetVal> Val = IntTranslator_c<aRetVal>().get_value(aStr);   \
		if (!Val) throw InvalidConversion_x(#aRetVal,aStr);                          \
		return Val.get();                                                            \
	}

FROM_STRING( uint8_t)
FROM_STRING(  int8_t)
FROM_STRING(uint16_t)
FROM_STRING( int16_t)
FROM_STRING(uint32_t)
FROM_STRING( int32_t)
FROM_STRING(uint64_t)
FROM_STRING( int64_t)

#undef FROM_STRING

template <> inline bool FromString<bool>(const std::string &aStr) {
	boost::optional<bool> Val = BoolTranslator_c().get_value(aStr);
	if (!Val) throw InvalidConversion_x("bool",aStr);
	return Val.get();
}

struct CCombinedAddr_t {
	CAddr_t Addr;
	bool IsParcelAddr;
};

template <> inline CCombinedAddr_t FromString<CCombinedAddr_t>(const std::string &aStr) {
	CCombinedAddr_t RetVal;
	try {
		RetVal.Addr = FromString(aStr, StringFormat_e::ProgramAddr);
		RetVal.IsParcelAddr = true;
	}
	catch (StringFormatError_x &) {
		RetVal.Addr = FromString(aStr, StringFormat_e::DataAddr);
		RetVal.IsParcelAddr = false;
	}
	return RetVal;
}

//template <> inline CAddr_t FromString<CAddr_t>(const std::string &aStr) {
//	return FromString(aStr, StringFormat_e::DataAddr);
//}

class InvalidParameter_x: public boost::property_tree::ptree_error {
public:
	explicit InvalidParameter_x(const std::string &aError): boost::property_tree::ptree_error(aError) {}
	explicit InvalidParameter_x(const boost::format &aFormat): boost::property_tree::ptree_error(aFormat.str()) {}
};


//#define DO_OPTIONS_LOG(a) a
#define DO_OPTIONS_LOG(a)

class Configuration_c {
public:
	Configuration_c() : mTree(nullptr), mOwner(false) {}
	Configuration_c(Configuration_c && aOri) : mTree(aOri.mTree), mOwner(aOri.mOwner) {}
	Configuration_c(const boost::property_tree::ptree &aTree) : mTree(&aTree), mOwner(false) {}
	virtual ~Configuration_c() {
		if (mOwner) delete mTree;
	}
	template <typename tRetVal> boost::optional<tRetVal> get_optional(const char *aPath) const { DO_OPTIONS_LOG(cerr << "       " << aPath << " - optional" << endl); return mTree->get_optional<tRetVal>(aPath); }
	template <typename tRetVal> tRetVal get(const char *aPath) const { DO_OPTIONS_LOG(cerr << "       " << aPath << endl); return mTree->get<tRetVal>(aPath); }
	template <typename tRetVal> tRetVal get(const char *aPath, const tRetVal aDefault) const { DO_OPTIONS_LOG(cerr << "       " << aPath << " - default: " << aDefault << endl); return mTree->get<tRetVal>(aPath, aDefault); }
	Configuration_c get_child(const char *aPath) const { DO_OPTIONS_LOG(cerr << "Enumerating children of " << aPath << endl); return Configuration_c(mTree->get_child(aPath)); }
	Configuration_c get_child_safe(const char *aPath) const {
		DO_OPTIONS_LOG(cerr << "Enumerating children of " << aPath << endl);
		if (mTree->get_optional<std::string>(aPath).is_initialized()) {
			return Configuration_c(mTree->get_child(aPath));
		}
		return GetEmptyConfig();
	}
	typedef boost::property_tree::ptree::const_iterator const_iterator;
	typedef boost::property_tree::ptree::const_assoc_iterator const_assoc_iterator;
	const_iterator begin() const { return mTree->begin(); }
	const_iterator end() const { return mTree->end(); }
	const_assoc_iterator find(const char *aKey) const { DO_OPTIONS_LOG(cerr << "Finding key: " << aKey << endl); return mTree->find(aKey); }

	bool empty() const { return mTree->empty(); }
	void Read(const char *aFileName) {
		if (mOwner) throw Generic_x("Can't read into const Configuration_c");
		mOwner = true;
		mTree = new boost::property_tree::ptree;
		boost::property_tree::read_info(aFileName, *const_cast<boost::property_tree::ptree *>(mTree));
	}
	static Configuration_c GetEmptyConfig() { return Configuration_c(mEmptyTree); } // return an empty tree that will support iterators but will have no children
protected:
	mutable const boost::property_tree::ptree *mTree;
	bool mOwner;
	static boost::property_tree::ptree mEmptyTree;
};

#undef DO_OPTIONS_LOG

namespace boost {
namespace property_tree {

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, bool> {
    typedef BoolTranslator_c type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, uint8_t> {
    typedef IntTranslator_c<uint8_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, int8_t> {
    typedef IntTranslator_c<int8_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, uint16_t> {
    typedef IntTranslator_c<uint16_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, int16_t> {
    typedef IntTranslator_c<int16_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, uint32_t> {
    typedef IntTranslator_c<uint32_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, int32_t> {
    typedef IntTranslator_c<int32_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, uint64_t> {
    typedef IntTranslator_c<uint64_t> type;
};

template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, int64_t> {
    typedef IntTranslator_c<int64_t> type;
};
/****
template<typename Ch, typename Traits, typename Alloc> struct translator_between<std::basic_string< Ch, Traits, Alloc >, CAddr_t> {
	typedef CAddrTranslator_c type;
};
****/

} // namespace property_tree
} // namespace boost


#endif // __CONFIG_FILE_H__
