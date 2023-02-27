#include "Parser.h"

#include "cppast/cpp_entity_index.hpp"
#include "cppast/parser.hpp"
#include "type_safe/reference.hpp"

#include "Generators/SerializeGenerator.h"
#include "Utils/Utils.h"

Parser::Parser(
	const std::string& searchPathCfgFile,
	const std::string& srcConfigFile,
	const std::string& sourceRootPath,
	const std::string& outputPath, 
	const std::string& templateDir)
{

	std::string tmpStr = Utils::LoadFile(searchPathCfgFile);
	mSearchPaths = Utils::Split(tmpStr, ";");
	tmpStr = Utils::LoadFile(srcConfigFile);
	mSrcFilePaths = Utils::Split(tmpStr, ";");
	Prepare();

	m_pSerializeGen = new SerializeGenerator(sourceRootPath, outputPath);
	m_pSerializeGen->SetTemplateFile(templateDir);
}

Parser::~Parser()
{
	if (m_pSerializeGen)
	{
		delete m_pSerializeGen;
		m_pSerializeGen = nullptr;
	}
}

void Parser::Prepare()
{
	// the compile_flags are generic flags
	cppast::compile_flags flags;
	flags |= cppast::compile_flag::ms_extensions;
	flags |= cppast::compile_flag::ms_compatibility;

	mConfig.set_flags(cppast::cpp_standard::cpp_17, flags);

	for (auto& path : mSearchPaths)
	{
		mConfig.add_include_dir(path);
	}
}

void Parser::GetReflectionEntities(const std::string& srcFile)
{
	// the logger is used to print diagnostics
	cppast::stderr_diagnostic_logger logger;

	// the entity index is used to resolve cross references in the AST
	// we don't need that, so it will not be needed afterwards
	cppast::cpp_entity_index idx;
	// the parser is used to parse the entity
	// there can be multiple parser implementations
	cppast::libclang_parser parser(type_safe::ref(logger));
	// parse the file
	auto file = parser.parse(idx, srcFile, mConfig);
	if (file)
	{
		m_pSerializeGen->GenCode(*file, srcFile);
	}
}

void Parser::GenerateFile()
{
	m_pSerializeGen->Finish();
}

void Parser::StartParse()
{
	for (auto& str : mSrcFilePaths)
	{
		std::filesystem::recursive_directory_iterator itr(str);
		for (auto& f : itr)
		{
			if (f.is_directory())
				continue;
			/*if (f.path().parent_path().generic_string().find("_generated") != std::string::npos)
			{
				continue;
			}*/
			if (f.path().filename().generic_string().rfind(".h") == std::string::npos)
				continue;
			/*if (f.path().generic_string().find("Data") == std::string::npos)
				continue;*/
			std::cout << "process file: " << f.path() << std::endl;
			//std::string path = f.path().generic_string();
			//std::replace(path.begin(), path.end(), std::string("\\\\"), std::string("/"));
			GetReflectionEntities(f.path().generic_string());
		}
	}
}