#pragma once

#include <string_view>
#include <unordered_map>
#include <tuple>

namespace FakeReal::reflection
{
    template <typename T>
    constexpr std::string_view type_name()
    {
#if defined(__clang__)
        constexpr std::string_view prefix = "std::string_view FakeReal::reflection::type_str() [T =  ";
        constexpr std::string_view suffix = "]";
#elif defined(_MSC_VER)
        constexpr std::string_view prefix = "class std::basic_string_view<char, struct std::char_traits<char>> __cdecl FakeReal::reflection::type_str<";
        constexpr std::string_view suffix = ">(void)";
#else
        constexpr std::string_view prefix =
        "constexpr std::string_view FakeReal::reflection::type_str() [with T = ";
        constexpr std::string_view suffix = "; std::string_view = std::basic_string_view<char>]";
#endif
        auto sig = std::string_view{ __PRETTY_FUNCTION__ };
        sig.remove_prefix(prefix.size());
        sig.remove_suffix(suffix.size());
        return sig;
    }

    template <typename T>
    constexpr std::string_view decay_type_name()
    {
        return type_name<std::decay_t<T>>();
    }
} // namespace FakeReal::reflection

struct Field
{
    constexpr Field(const std::string_view name, const std::string_view type, uint32_t offset)
    : mName(name), mType(type), mOffset(offset)
    {}

    const std::string_view mName;
    const std::string_view mType;
    uint32_t mOffset;
};

template <int N>
struct FieldWithMeta : public Field
{
    constexpr FieldWithMeta(const std::string_view name, const std::string_view type, uint32_t offset,
                            const std::unordered_map<std::string_view, std::string_view>& m)
        : Field(name, type, offset)
        , mMetas(m)
    {
    }

    constexpr int MetaCount() const
    {
        return N;
    }

    const std::unordered_map<std::string_view, std::string_view>& mMetas;
};

template <>
struct FieldWithMeta<0> : public Field
{
    constexpr FieldWithMeta(const std::string_view name, const std::string_view type, uint32_t offset)
        : Field(name, type, offset)
    {
    }

    constexpr int MetaCount() const
    {
        return 0;
    }
};

template <typename T>
struct ClassInfo
{
    inline static const constexpr std::string_view GetClassName()
    {
        return FakeReal::reflection::decay_type_name<T>();
    }

    inline static const constexpr auto AllFields() { return std::make_tuple(); }
};

#define HAS_MEMBER(s)\
template<typename ____TTTT>\
struct HasMember_##s{\
    template <typename T_T>\
    static auto check(T_T)->typename std::decay<decltype(T_T::s)>::type;\
    static void check(...);\
    using type = decltype(check(std::declval<____TTTT>()));\
    enum{value=!std::is_void<type>::value};\
};
/*
    auto check(T_T)->typename std::decay<decltype(T_T::s)>::type 是一种类型萃取的方法，可以获得返回类型的decay值.
    所谓的decay值就是去除引用以及const等特性，获得一个被削弱的返回值。
*/

HAS_MEMBER(AllFields)
/*
template <typename ____TTTT>
struct HasMember_AllFields
{
    template <typename T_T>
    static auto check(T_T) -> typename std::decay<decltype(T_T::AllFields)>::type;
    static void check(...);
    using type = decltype(check(std::declval<____TTTT>()));
    enum
    {
        value = !std::is_void<type>::value
    };
};
*/

template <typename Tuple, typename F, std::size_t... Indices>
constexpr void for_each_impl(Tuple&& tuple, F&& f, std::index_sequence<Indices...>)
{
    using swallow = int[];
    (void)swallow{ 1, (f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int{})... };
}

template <typename Fn, typename Tuple>
inline constexpr void ForEachTuple(Tuple&& tuple, Fn&& fn)
{

    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
    for_each_impl(std::forward<Tuple>(tuple), std::forward<Fn>(fn),
                  std::make_index_sequence<N>{});
}

template <bool atomic, typename V, typename Fn, typename Tuple>
inline static constexpr void __for_each_field_impl(Tuple&& tup, V&& value, Fn&& fn)
{
    ForEachTuple(std::forward<Tuple>(tup),
                 [&value, &fn](auto&& field_schema) {
                     using ClassName_C = std::decay_t<decltype(value.*(field_schema.ptr))>;
                     if constexpr (!isAtomic<ClassName_C>() && atomic)
                     {
                         static_assert(HasMember_AllFields<ClassInfo<ClassName_C>>::value);
                         SClass<ClassName_C>::ForEachFieldAtomic(std::forward<ClassName_C>(value.*(field_schema.ptr)), fn);
                         constexpr int leng = std::tuple_size(ClassInfo<ClassName_C>::all_static_fields());
                         if constexpr (leng > 0)
                         {
                             static_assert(has_member_all_static_fields<ClassInfo<ClassName_C>>::value);
                             SClass<ClassName_C>::ForEachStaticFieldAtomic(fn);
                         }
                     }
                     else
                         fn(value.*(field_schema.ptr), field_schema.info);
                 });
}

template <typename T>
struct FClass
{
    using ClassName = std::decay_t<T>;
    using info = ClassInfo<ClassName>;
    inline static const constexpr char* GetName() { return info::GetClassName(); }

    template<typename Fn>
    inline static constexpr void ForeachFields(Fn&& func)
    {
        if constexpr (HasMember_AllFields<info>::value)
        {
            func();
        }
    }
};