#include <iostream>
#include <string>
#include "FbxConverter.h"

int main(int argc, char** argv)
{
	std::string srcFile;
	std::string destFile;
	std::string argstr;
	FBXSDK_printf("\n\nFBXģ���ļ�:\n\n");
	std::getline(std::cin, srcFile);
	FBXSDK_printf("\n\n����ģ���ļ�:\n\n");
	std::getline(std::cin, destFile);
	FBXSDK_printf("\n\n����ģ�Ͳ���:\n\n");
	std::getline(std::cin, argstr);
	char* args[3];
	args[0] = "C:\\Dev\\FakeReal2\\asset\\objects\\sponza\\Sponza.fbx";
	args[1] = "C:\\Dev\\FakeReal2\\asset\\objects\\sponza\\Sponza_.json";
	args[2] = "-s";

	FakeReal::FbxConverter converter(3, args);
	converter.ExportFile();

	std::cin.get();
	return 0;
}