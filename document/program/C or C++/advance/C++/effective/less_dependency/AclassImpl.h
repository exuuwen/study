#ifndef _H_ACLASSIMPL_H_
#define _H_ACLASSIMPL_H_

#include <string>
#include <tr1/memory>
#include "Bclass.h"

class AclassImpl 
{
public:
	AclassImpl(const std::string &s = "", int data = 0);
	void interface_1();
	std::string interface_2();
    // .. 
private:
    //  implementation details are leaking as below.. 
	std::string internalData_1;
	Bclass internalData_2;
}; 
#endif
