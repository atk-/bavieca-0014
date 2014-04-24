#include <windows.h>
#include <iostream>
#include "BaviecaAPI.h"

int main() {
	HINSTANCE hDLL = LoadLibrary("newbavieca.dll");
	
	if ( !hDLL ) {
		std::cout << "loading failed" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::cout << "loading succeeded!" << std::endl;
	std::cout << "constructing a BaviecaAPI object..." << std::endl;
	
	const char* configFile = "bavieca.cfg";
	Bavieca::BaviecaAPI *api = Bavieca::BaviecaAPI::create(configFile, 2, NULL, 0);
	
	return 0;
}