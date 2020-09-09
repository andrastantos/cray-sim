#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "parser.h"

class CmdFactoryBase_i {
public:
	virtual ~CmdFactoryBase_i() {}
	virtual bool ParseAndExec(TokenStream_t::const_iterator aBegin, TokenStream_t::const_iterator aEnd) = 0;
	virtual std::string GetCommandName() const = 0;
	virtual std::string GetDeviceName() const = 0;
};

#endif // __COMMANDS_H__
