#ifndef _H_ACLASS_H_
#define _H_ACLASS_H_

#include <string>
#include <tr1/memory>

class AclassImpl;

class Aclass 
{
public:
	Aclass(const std::string& s, int data);
	void interface_1();
	std::string interface_2();
private:
	std::tr1::shared_ptr<AclassImpl> pImpl;
}; 

#endif
