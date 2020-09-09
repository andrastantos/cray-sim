#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>

class Generic_x: public std::exception {
public:
	Generic_x() {}
	explicit Generic_x(const std::string &aError) { mErrorStrm << aError; mSpacePrinted = (aError.length() == 0); }
	Generic_x(const Generic_x &aEx) : mErrorStrm(aEx.mErrorStrm.str()), mErrorStr(aEx.mErrorStr), mSpacePrinted(aEx.mSpacePrinted) {}
	virtual ~Generic_x() throw() {}
	virtual const char *what() const throw() override { mErrorStr = mErrorStrm.str(); return mErrorStr.c_str(); }

	template <typename tElement> Generic_x &operator << (const tElement &aElement) {
		if (!mSpacePrinted) {
			mErrorStrm << " ";
			mSpacePrinted = true;
		}
		mErrorStrm << aElement;
		return *this;
	};
protected:
	void ResetSpacePrinted() { mSpacePrinted = false; }
	std::stringstream mErrorStrm;
	mutable std::string mErrorStr;
	bool mSpacePrinted;
};

class Assert_x: public Generic_x {
public:
	explicit Assert_x(const std::string &aError): Generic_x(aError) {}
};

class InvalidConversion_x: public std::exception {
public:
	InvalidConversion_x(const char *aType, const std::string &aValue) {
		std::stringstream Strm;
		Strm << "Cannot convert " << aValue << " to type " << aType;
		mMessage = Strm.str();
	}
	virtual const char *what() const throw() override {
		return mMessage.c_str();
	}
private:
	std::string mMessage;
};

#endif // __EXCEPTIONS_H__
