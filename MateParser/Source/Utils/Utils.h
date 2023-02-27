#pragma once
#include <string>
#include <filesystem>
#include <stack>
#include <fstream>
#include <algorithm>

namespace Utils
{
	static std::string MakeRelativePath(const std::string& rootPath, const std::string& path)
	{
		std::filesystem::path root	= rootPath;
		std::filesystem::path tmp	= path;
		std::stack<std::string> folderNames;
		while (tmp != root)
		{
			if (tmp == std::filesystem::current_path().root_path() || tmp.parent_path() == root)
			{
				break;
			}
			folderNames.push(tmp.parent_path().filename().generic_string());
			tmp = tmp.parent_path();
		}
		std::string relativePath = "";
		while (!folderNames.empty())
		{
			relativePath += folderNames.top() + "/";
			folderNames.pop();
		}
		return relativePath;
	}

	static void SaveFile(const std::string& bufferStr, const std::string& filePath, const std::string& fileName)
	{
		std::filesystem::path outPath(filePath);
		if (!std::filesystem::exists(outPath))
		{
			std::filesystem::create_directory(outPath);
		}
		std::ofstream outFile(filePath + fileName);
		if (outFile.is_open())
		{
			outFile << bufferStr << std::endl;
			outFile.flush();
			outFile.close();
		}
	}

	static std::string LoadFile(const std::string& fileName)
	{
		std::ifstream inFile(fileName);
		std::stringstream ss;
		if (inFile.is_open())
		{
			ss << inFile.rdbuf();
			inFile.close();
			return ss.str();
		}
		return std::string();
	}

	static std::vector<std::string> Split(std::string input, std::string pat)
	{
		std::vector<std::string> ret_list;
		while (true)
		{
			size_t index = input.find(pat);
			std::string sub_list = input.substr(0, index);
			if (!sub_list.empty())
			{
				ret_list.push_back(sub_list);
			}
			input.erase(0, index + pat.size());
			if (index == -1)
			{
				break;
			}
		}
		return ret_list;
	}
}