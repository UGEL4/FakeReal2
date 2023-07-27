//#include "Parser/Parser.h"

#include "Refl/reflection_rule.h"

struct TestComponent
{
    [[refl]] [[meta("SaeruHikari")]] float attrib = 123.f;
};

template<>
struct ClassInfo<TestComponent>
{
	static constexpr auto attribMeta = unordered_map_c<std::string_view, std::string_view>(
		{{"meta", "SaeruHikari"}}
	);
	FIELD_INFO(attrib, TestComponent, attribMeta);
};

int main(int argc, char** argv)
{
	/* if (argc < 7)
	{
		std::cout << "invalid arguments count!" << std::endl;
		return 0;
	} */
	/*Parser pa(
		"F:/FakeReal/FakeReal2/FakeReal2/Config/search_paths.json",
		"F:/FakeReal/FakeReal2/FakeReal2/Config/source_file_path.json",
		"F:/FakeReal/FakeReal2/FakeReal2/FakeRealRuntime/Source",
		"F:/FakeReal/FakeReal2/FakeReal2/FakeRealRuntime/Source/_generated",
		"F:/FakeReal/FakeReal2/FakeReal2/MateParser/template"
	);*/
	/* std::cout << "---------------start parse " << argv[6] << "-----------------" << std::endl;
	Parser pa(argv[1], argv[2], argv[3], argv[4], argv[5]);
	pa.StartParse();
	pa.GenerateFile();
	std::cout << "---------------end parse-----------------" << std::endl; */
	return 0;
}