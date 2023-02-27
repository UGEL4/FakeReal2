#include "SerializeGenerator.h"
#include <cppast/visitor.hpp>
#include <cppast/cpp_member_variable.hpp>
#include <fstream>
#include "Utils/Utils.h"

void SerializeGenerator::SetTemplateFile(const std::string& directory)
{
	mCommonTemplateFile			= directory + "/CommonHeadFileGen.mustache";
	mSerializeTemplateFile		= directory + "/allSerialize.cpp.mustache";
	mSerializeHeadTemplateFile	= directory + "/allSerialize.h.mustache";
}

void SerializeGenerator::GenCode(const cppast::cpp_file& cppFile, const std::string& srcFile)
{
	static std::string s_genFileSufixStr = "_gen.h";

	Mustache::data ClassDefineHeadFiles = Mustache::data::type::list;
	Mustache::data classDefines			= Mustache::data::type::list;
	bool genSrcHeadFile					= false;

	cppast::visit(
		cppFile,
		[&](const cppast::cpp_entity& e, const cppast::visitor_info& info)
		{
			if (e.kind() == cppast::cpp_entity_kind::class_t && cppast::is_definition(e) && !info.is_old_entity()
				&& cppast::has_attribute(e, "RefrectionClass"))
			{
				genSrcHeadFile	= true;
				auto& class_	= static_cast<const cppast::cpp_class&>(e);
				Mustache::data classDefine;
				classDefine.set("ClassName", class_.name());

				// serialize base classes
				Mustache::data baseClassDefines = Mustache::data::type::list;
				for (auto& base : class_.bases())
				{
					if (cppast::has_attribute(base, "RefrectionClass"))
					{
						Mustache::data baseClassDefine;
						baseClassDefine.set("BaseClassName", base.name());
						baseClassDefines.push_back(baseClassDefine);
					}
				}

				//member
				static std::string vectorPrefix = "std::vector<";
				bool hadSetArrayLength			= false;
				Mustache::data fieldList		= Mustache::data::type::list;
				for (auto& member : class_)
				{
					if (member.kind() == cppast::cpp_entity_kind::member_variable_t)
					{
						auto& var = static_cast<const cppast::cpp_member_variable&>(member);
						if (cppast::has_attribute(e, "WhiteList"))
						{
							//打上了 PROPERTY 标记的成员变量会被序列化
							if (cppast::has_attribute(var, "Property"))
							{
								Mustache::data field;
								field.set("ClassFieldName", var.name());
								if (auto attr = cppast::has_attribute(var, "DisplayName"))
								{
									field.set("FieldDisplayName", attr.value().arguments().value().as_string());
								}
								else
								{
									field.set("FieldDisplayName", var.name());
								}

								bool isVector = false;
								code_generator gen(var);
								const std::string& varTypeStr = gen.str();
								if (varTypeStr.find(vectorPrefix) != std::string::npos)
								{
									isVector = true;
									if (!hadSetArrayLength)
									{
										hadSetArrayLength = true;
										field.set("ArrayLength", true);
									}
								}
								field.set("ClassFieldIsVector", isVector);

								fieldList.push_back(field);
							}
						}
						else
						{
							Mustache::data field;
							field.set("ClassFieldName", var.name());
							if (auto attr = cppast::has_attribute(var, "DisplayName"))
							{
								field.set("FieldDisplayName", attr.value().arguments().value().as_string());
							}
							else
							{
								field.set("FieldDisplayName", var.name());
							}

							bool isVector = false;
							code_generator gen(var);
							const std::string& varTypeStr = gen.str();
							if (varTypeStr.find(vectorPrefix) != std::string::npos)
							{
								isVector = true;
								if (!hadSetArrayLength)
								{
									hadSetArrayLength = true;
									field.set("ArrayLength", true);
								}
							}
							field.set("ClassFieldIsVector", isVector);

							fieldList.push_back(field);
						}
					}
				}
				if (!baseClassDefines.is_empty_list())
				{
					classDefine.set("BaseClassDefines", baseClassDefines);
				}
				if (!fieldList.is_empty_list())
				{
					classDefine.set("ClassFieldDefines", fieldList);
				}

				classDefines.push_back(classDefine);
				mClassDefines.push_back(classDefine);
			}
		}
	);

	if (genSrcHeadFile)
	{
		//头文件路径
		std::filesystem::path srcFilePath(srcFile);
		std::string headFileName = srcFilePath.stem().generic_string() + s_genFileSufixStr;
		mClassIncludeHeads.push_back(Mustache::data("HeadFileName", headFileName));//*_gen.h文件和AllClassSerialize.cpp位于文件同一个目录

		//生成srcFile_gen.h
		ClassDefineHeadFiles.push_back(Mustache::data("HeadFileName", Utils::MakeRelativePath(mSrcIncludeRootPath, srcFile) + srcFilePath.filename().generic_string()));
		Mustache::data HeadFileRenderData;
		HeadFileRenderData.set("IncludeHeadFiles", ClassDefineHeadFiles);
		HeadFileRenderData.set("ClassDefines", classDefines);
		std::string srcFile_gen = srcFilePath.stem().generic_string() + "_gen.h";
		RnederCommonHeadFile(HeadFileRenderData, srcFile_gen);

		mAllHeadFileDefines.push_back(Mustache::data("HeadFileName", srcFile_gen));
	}
}

void SerializeGenerator::Finish()
{
	std::string view = Utils::LoadFile(mSerializeTemplateFile);
	Mustache::mustache tmp(view);
	Mustache::data renderData;
	renderData.set("ClassDefines", mClassDefines);
	renderData.set("IncludeHeadFiles", mClassIncludeHeads);
	std::string fileStr = tmp.render(renderData);
	//save
	Utils::SaveFile(fileStr, mOutputDir, "/AllClassSerialize.cpp");

	view = Utils::LoadFile(mSerializeHeadTemplateFile);
	Mustache::mustache headTmp(view);
	Mustache::data headRenderData;
	headRenderData.set("IncludeHeadFiles", mAllHeadFileDefines);
	fileStr = headTmp.render(headRenderData);
	//save
	Utils::SaveFile(fileStr, mOutputDir, "/AllSerialize.h");
}

void SerializeGenerator::RnederCommonHeadFile(const Mustache::data& data, const std::string& fileName)
{
	std::string view = Utils::LoadFile(mCommonTemplateFile);
	Mustache::mustache tmp(view);
	std::string fileStr = tmp.render(data);

	//save
	Utils::SaveFile(fileStr, mOutputDir, "/" + fileName);
}
