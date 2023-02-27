#pragma once
#include <mustache.hpp>
#include <cppast/cpp_file.hpp>
#include <cppast/code_generator.hpp>
#include <string>
#include <stack>
#include <filesystem>

namespace Mustache = kainjow::mustache;

class Generator
{
public:
	Generator() {}
	~Generator() {}

	virtual void SetTemplateFile(const std::string& directory) = 0;
	virtual void GenCode(const cppast::cpp_file& cppFile, const std::string& srcFile) = 0;
	virtual void Finish() = 0;
};

//copy from cppast example
class code_generator : public cppast::code_generator
{
	std::string str_;                 // the result
	bool was_newline_ = false; // whether or not the last token was a newline
	// needed for lazily printing them

public:
	code_generator(const cppast::cpp_entity& e)
	{
		// kickoff code generation here
		cppast::generate_code(*this, e);
	}

	// return the result
	const std::string& str() const noexcept
	{
		return str_;
	}

private:
	// called to retrieve the generation options of an entity
	generation_options do_get_options(const cppast::cpp_entity&,
		cppast::cpp_access_specifier_kind) override
	{
		// generate declaration only
		return code_generator::declaration;
	}

	// no need to handle indentation, as only a single line is used
	void do_indent() override {}
	void do_unindent() override {}

	// called when a generic token sequence should be generated
	// there are specialized callbacks for various token kinds,
	// to e.g. implement syntax highlighting
	void do_write_token_seq(cppast::string_view tokens) override
	{
		if (was_newline_)
		{
			// lazily append newline as space
			str_ += ' ';
			was_newline_ = false;
		}
		// append tokens
		str_ += tokens.c_str();
	}

	// called when a newline should be generated
	// we're lazy as it will always generate a trailing newline,
	// we don't want
	void do_write_newline() override
	{
		was_newline_ = true;
	}
};