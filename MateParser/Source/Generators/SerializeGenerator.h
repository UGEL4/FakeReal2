#pragma once
#include "Generator.h"

class SerializeGenerator : public Generator
{
public:
	SerializeGenerator(const std::string rootPath, const std::string& outputDir)
		: mSrcIncludeRootPath(rootPath), mOutputDir(outputDir)
	{}
	~SerializeGenerator() {}

	virtual void SetTemplateFile(const std::string& directory) override;
	virtual void GenCode(const cppast::cpp_file& cppFile, const std::string& srcFile) override;
	virtual void Finish() override;

private:
	void RnederCommonHeadFile(const Mustache::data& data, const std::string& fileName);

private:
	std::string mSrcIncludeRootPath;//F:\FakeReal\FakeReal2\FakeReal2\FakeRealRuntime\Source
	std::string mRootPath; //项目根路径， 从根路径开始，保存生成的代码
	std::string mOutputDir;

	Mustache::data mClassDefines { Mustache::data::type::list };
	Mustache::data mClassIncludeHeads { Mustache::data::type::list };
	Mustache::data mAllHeadFileDefines { Mustache::data::type::list };

	std::string mCommonTemplateFile;
	std::string mSerializeTemplateFile;
	std::string mSerializeHeadTemplateFile;
};