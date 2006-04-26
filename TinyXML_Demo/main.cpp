#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <TinyXML\tinyxml.h>
#include <TinyXML\tinystr.h>

#define			XML_FILE_PATH		"SlsConfig.xml"

// 
// Main Function
// 

int main(int argc, char ** argv)
{
	TiXmlDocument doc (XML_FILE_PATH);
	
	bool isLoadOK = doc.LoadFile();
	if ( !isLoadOK ) 
	{
		printf("Invalid XML File Path: %s\n", XML_FILE_PATH);
		
		system("pause");
		exit(0);
	}

	const TiXmlElement * rootElement = doc.RootElement();
	if (strcmp(rootElement->Value(), "SLSConfiguration") != 0) {
		printf("Invalid XML File Root Element: %s\n", rootElement->Value());

		system("pause");
		exit(0);
	}

	return EXIT_SUCCESS;
}