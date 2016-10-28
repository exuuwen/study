#include "XmlParser.h"
#include <assert.h>
#include <string.h>

unsigned int XmlParser::count_ = 0;

XmlParser::XmlParser() 
{
	if(count_ == 0)
	{
		xmlInitParser();
		LIBXML_TEST_VERSION;
	}
	
	count_++;
}

XmlParser::~XmlParser() 
{
	xmlXPathFreeContext(xpathCtx_);
	xmlFreeDoc(xmlDoc_);
	
	count_--;
	if (count_ == 0)
	{
		xmlCleanupParser();
	}
}

int XmlParser::saveAsFile(const char * filename)
{
	int ret = xmlSaveFile(filename, xmlDoc_);
	if(ret < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

char* XmlParser::getRootNodeName()
{
	char* str = NULL;
	xmlNodePtr root = xmlDocGetRootElement(xmlDoc_);
	if(root)
	{
		str = strdup((char*)root->name);
	}

	return str;
}

char* XmlParser::getTextByXPath(const char* xpathExpr) const
{
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *)xpathExpr, xpathCtx_);
	if (xpathObj == NULL || xpathObj->nodesetval == NULL || xpathObj->nodesetval->nodeTab == NULL)
	{
		xmlXPathFreeObject(xpathObj);
		return NULL;
	}
	//TODO:maybe it is not the first chid
	xmlNodePtr n = xpathObj->nodesetval->nodeTab[0]->children;
	char *str = NULL;	
	if (xmlNodeIsText(n)) 
	{
		if (n->content) 
		{
			str = strdup((const char*)n->content);
		}
	}

	xmlXPathFreeObject(xpathObj);

    return str;
}

char* XmlParser::getPropertyByXPath(const char * xpathExpr, const char * propertyName) const
{
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *)xpathExpr, xpathCtx_);
	if (xpathObj == NULL || xpathObj->nodesetval == NULL || xpathObj->nodesetval->nodeTab == NULL)
	{
		xmlXPathFreeObject(xpathObj);
		return NULL;
	}

	char* str = (char *)xmlGetProp(xpathObj->nodesetval->nodeTab[0], (const xmlChar *)propertyName);

	xmlXPathFreeObject(xpathObj);

	return str;
}

bool XmlParser::isExistByXPath(const char* xpathExpr) const
{
	xmlXPathObjectPtr xpathObj = xmlXPathEvalExpression((const xmlChar *)xpathExpr, xpathCtx_);
	if (xpathObj == NULL || xpathObj->nodesetval == NULL || xpathObj->nodesetval->nodeTab == NULL)
	{
		xmlXPathFreeObject(xpathObj);
		return false;
	}

	xmlXPathFreeObject(xpathObj);

	return true;
}

TextXmlParser::TextXmlParser(const char * xml)
{
	xmlDoc_ = xmlReadDoc((xmlChar *) xml, NULL, NULL, 0);
	assert(xmlDoc_ != NULL);

	xpathCtx_ = xmlXPathNewContext(xmlDoc_);
	assert(xpathCtx_ != NULL);
}

FileXmlParser::FileXmlParser(const char * filename)
{
	xmlDoc_ = xmlReadFile(filename, NULL, 0);
	assert(xmlDoc_ != NULL);

	xpathCtx_ = xmlXPathNewContext(xmlDoc_);
	assert(xpathCtx_ != NULL);
}

