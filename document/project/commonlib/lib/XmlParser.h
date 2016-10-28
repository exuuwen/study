#ifndef _XmlParser_H_
#define _XmlParser_H_

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

class XmlParser
{
public:
	int saveAsFile(const char* filename);

	char* getRootNodeName();
	char* getTextByXPath(const char * xpathExpr) const;
	char* getPropertyByXPath(const char * xpathExpr, const char * propertyName) const;
	bool isExistByXPath(const char * xpathExpr) const;

protected:
	XmlParser();
	~XmlParser();

	xmlDocPtr xmlDoc_;
	xmlXPathContextPtr xpathCtx_;
	static unsigned int count_;
};

class TextXmlParser : public XmlParser
{
public:
	TextXmlParser(const char *xml);
};

class FileXmlParser : public XmlParser
{
public:
	FileXmlParser(const char *filename);
};

#endif
