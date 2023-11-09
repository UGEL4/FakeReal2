#include <iostream>
#include "model_export.h"

int main(int argc, char** argv)
{
    std::cout << "assipm main." << std::endl;

    if (argc < 3)
    {
        std::cerr << "args less then 2." << std::endl;
        //return -1;
    }

    const char* inFile = argv[1];
    const char* outFile = argv[2];

    //ExportModel("C:/Dev/FakeReal2/asset/objects/sponza/Sponza_Modular.FBX", "C:/Dev/FakeReal2/asset/objects/sponza/Sponza_Modular.json");
    ExportModel("D:/c++/FakeReal2/asset/objects/sponza/Sponza_Modular.FBX", "D:/c++/FakeReal2/asset/objects/sponza/Sponza_Modular.json");
    //ExportModel("D:\\c++\\nanosuit\\nanosuit.obj", "D:\\c++\\nanosuit\\out\\nanosuit.json");
    //ExportModel(inFile, outFile);

    return 0;
}