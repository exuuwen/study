#include "Aclass.h"
#include <string>
#include <tr1/memory>
#include "AclassImpl.h"

Aclass::Aclass(const std::string& s, int data):pImpl(new AclassImpl(s, data)) 
{
    // .. 
}

void Aclass::interface_1()
{
	pImpl->interface_1();
}

std::string Aclass::interface_2()
{
	return pImpl->interface_2();
}
