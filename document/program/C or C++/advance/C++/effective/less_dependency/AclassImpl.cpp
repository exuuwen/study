#include "AclassImpl.h"
#include <stdio.h>
AclassImpl::AclassImpl(const std::string &s, int data):internalData_1(s), internalData_2(data)
{
}

void AclassImpl::interface_1()
{
	printf("interface_1 hahahah\n");
	internalData_2.show();
}

std::string AclassImpl::interface_2()
{
	return internalData_1;
}
