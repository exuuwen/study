#include "XmlParser.h"
#include <stdio.h>

#define TESTXML \
"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"\
"<root>"\
    "<from entity=\"2\"/>"\
    "<to entity=\"3\"/>"\
    "<request id=\"1\" action=\"201\">"\
        "<reply required=\"yes\">"\
            "<status/>"\
            "<result/>"\
        "</reply>"\
        "<parameters>"\
            "<instance id=\"1\" status=\"1\">"\
                "<name>uos</name>"\
                "<vcpu>1</vcpu>"\
                "<memory id=\"qw\">112111</memory>"\
                "<domain>234</domain>"\
	        "<mac>52:54:00:ae:fa:23</mac>"\
	        "<ip>10.21.1.21</ip>"\
            "</instance>"\
            "<appliance id=\"1\">"\
	        "<name>ubuntuServer.tar.gz</name>"\
	        "<uri>ftp://efuchxu:efuchxu@10.170.121.2/vm/ubuntuServer.tar.gz</uri>"\
	        "<checksum>8b2b2712863518b9f4b685e4152b354a</checksum>"\
            "</appliance>"\
            "<osmanager>"\
	        "<clc>"\
	            "<ip>10.12.12.12</ip>"\
	            "<port>2531</port>"\
	        "</clc>"\
	        "<tag>1</tag>"\
	        "<secret>2333</secret>"\
            "</osmanager>"\
            "<storage>"\
	        "<ip>10.23.3.21</ip>"\
	        "<method>1</method>"\
	        "<parm>parm</parm>"\
            "</storage>"\
        "</parameters>"\
    "</request>"\
"</root>"

int main()
{
	TextXmlParser *xp = new TextXmlParser(TESTXML);
	    
	char *s = xp->getRootNodeName();
	if(s != NULL)
	{
		printf("root name :%s\n", s);
		free(s);
	}

	s = xp->getTextByXPath("/root/request/parameters/instance/memory");
	if(s != NULL)
	{
		printf("/root/request/parameters/instance/memory:%s\n", s);
		free(s);
	}
	
	s = xp->getPropertyByXPath("root/request/parameters/instance/memory", "id");
	if(s != NULL)
	{
		printf("/root/request/parameters/instance/memory, id :%s\n", s);
		free(s);
	}

	bool a = xp->isExistByXPath("/root/request/reply/result");
	if(a == true)
	{
		printf("/root/request/reply/result exist\n");
	}
	else
	{
		printf("/root/request/result do not exist\n");
	}

	int ret = xp->saveAsFile("store.xml");
	if(ret < 0)
	{
		printf("store xml fail\n");
		return -1;
	}

	FileXmlParser txp("store.xml");

	s = txp.getRootNodeName();
	if(s != NULL)
	{
		printf("root name :%s\n", s);
		free(s);
	}

	s = txp.getTextByXPath("/root/request/parameters/instance/memory");
	if(s != NULL)
	{
		printf("/root/request/parameters/instance/memory:%s\n", s);
		free(s);
	}
	
	s = txp.getPropertyByXPath("root/request/parameters/instance/memory", "id");
	if(s != NULL)
	{
		printf("/root/request/parameters/instance/memory, id :%s\n", s);
		free(s);
	}

	a = txp.isExistByXPath("/root/request/reply/result");
	if(a == true)
	{
		printf("/root/request/reply/result exist\n");
	}
	else
	{
		printf("/root/request/result do not exist\n");
	}

	ret = txp.saveAsFile("store_tmp.xml");
	if(ret < 0)
	{
		printf("store xml fail\n");
		return -1;
	}

	delete xp;

	return 0;
}
