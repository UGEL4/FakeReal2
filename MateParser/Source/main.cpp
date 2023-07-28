//#include "Parser/Parser.h"

#include "Refl/reflection_rule.h"
#include <iostream>

struct TestComponent
{
    [[refl]] [[meta("SaeruHikari")]] float attrib = 123.f;
	static constexpr const float staticAttrib = 456.f;

	void Method(int param) const
	{
		std::cout << "void TestComponent::Method(int param) const : param = " << param << std::endl;
	}
};

template <>
struct ClassInfo<TestComponent>
{
	inline static const constexpr std::string_view GetClassName()
    {
        return FakeReal::reflection::decay_type_name<TestComponent>();
    }

    static constexpr auto attribMeta = unordered_map_c<std::string_view, std::string_view>(
    { { "meta", "SaeruHikari" } });

    static constexpr auto staticAttribMeta = unordered_map_c<std::string_view, std::string_view>(
    { { "meta", "This is a static attribute" } });

	static constexpr auto MethodMeta = unordered_map_c<std::string_view, std::string_view>(
    { { "meta", "This is a Method" } });

    FIELD_INFO(attrib, TestComponent, attribMeta);
    STATIC_FIELD_INFO(staticAttrib, TestComponent, staticAttribMeta);
	METHOD_INFO(Method, TestComponent, MethodMeta);

    inline static const constexpr auto AllFields()
    {
        return std::make_tuple(attribInfo());
    }

    inline static const constexpr auto AllStaticFields()
    {
        return std::make_tuple(staticAttribInfo());
    }

    inline static const constexpr auto AllMethod()
    {
		return std::make_tuple(MethodInfo());
    }
};

//HAS_MEMBER(attrib);

template <typename T>
void auto_func_template(T&& field, const Field& meta, const std::string_view prefix)
{
    using TT = std::decay_t<decltype(field)>;
    if constexpr (IsAtomic<TT>())
        std::cout << prefix << ": " << meta.mType << " - " << meta.mName << std::endl;
    else if constexpr (HasMember_AllFields<ClassInfo<TT>>::value)
    {
        std::cout << "FOREACH OF CHILD FIELD:\n";
        FClass<TT>::ForEachField(field,
                                 [&](auto&& _field, auto&& _meta) {
                                     auto_func_template(_field, _meta, meta.mName);
                                 });
        std::cout << "END FOREACH OF CHILD FIELD\n";
    }
}

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
    TestComponent testComp;
	std::cout << "type name : " << FClass<std::decay<decltype(testComp)>::type>::GetName() << std::endl;
    FClass<std::decay<decltype(testComp)>::type>::ForeachField(testComp, [](auto&& field, auto&& meta) {
        auto_func_template(field, meta, "");
    });

	FClass<std::decay<decltype(testComp)>::type>::ForeachStaticField([](auto&& field, auto&& meta)
	{
		auto_func_template(field, meta, "");
	});

    FClass<std::decay<decltype(testComp)>::type>::ForeachMethod(testComp,
                                                                [&](auto&& method, auto&& meta) {
                                                                    if constexpr (std::is_same<typename std::decay<decltype(method)>::type,
																	void(TestComponent::*)(int) const>())
																	{
																		std::cout << meta.mType << " : " << meta.mName << std::endl;
																		(testComp.*method)(10);
																	}
                                                                });
    return 0;
}