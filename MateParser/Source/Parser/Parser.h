#pragma once

#include <string>
#include <mustache.hpp>
#include <vector>
#include "cppast/libclang_parser.hpp"

#include "Generators/Generator.h"

class Parser
{
public:
	Parser(
		const std::string& searchPathCfgFile,
		const std::string& srcConfigFile,
		const std::string& sourceRootPath,
		const std::string& outputPath, 
		const std::string& templateDir
	);
	~Parser();

private:
	void Prepare();

public:
	void GetReflectionEntities(const std::string& srcFile);
	void GenerateFile();

	Mustache::data mRenderData{ Mustache::data::type::list };
	Mustache::data mIncludeHeadDatas;//cpp包含有文件list

	void StartParse();
private:
	//std::string mSourcePath;
	std::string mOutputPath;
	std::vector<std::string> mIncludeFiles;

	std::vector<std::string> mSearchPaths;
	std::vector<std::string> mSrcFilePaths;

private:
	Generator* m_pSerializeGen { nullptr };

private:
	// the compile config stores compilation flags
	cppast::libclang_compile_config mConfig;
};
