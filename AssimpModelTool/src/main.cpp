#include <iostream>
#include "model_export.h"

int main(int argc, char** argv)
{
    std::cout << "assipm main." << std::endl;

    /* if (argc < 3)
    {
        std::cerr << "args less then 2." << std::endl;
    } */

    const char* inFile = argv[1];
    const char* outFile = argv[2];

    ExportModel("C:\\Dev\\nanosuit\\nanosuit.obj", "C:\\Dev\\nanosuit\\out\\nanosuit.json");
    //ExportModel("D:\\c++\\nanosuit\\nanosuit.obj", "D:\\c++\\nanosuit\\out\\nanosuit.json");

    return 0;
}