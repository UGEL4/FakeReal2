#include <iostream>
#include <string>
#include "FbxConverter.h"

int main(int argc, char** argv)
{
	std::string srcFile;
	std::string destFile;
	std::string argstr;
	FBXSDK_printf("\n\nFBX文件:\n\n");
	std::getline(std::cin, srcFile);
	FBXSDK_printf("\n\n导出文件:\n\n");
	std::getline(std::cin, destFile);
	FBXSDK_printf("\n\n导出参数:\n\n");
	std::getline(std::cin, argstr);
	char* args[3];
	args[0] = srcFile.data();
	args[1] = destFile.data();
	args[2] = argstr.data();

	FakeReal::FbxConverter converter(3, args);
	converter.ExportFile();

	std::cin.get();
	return 0;
}