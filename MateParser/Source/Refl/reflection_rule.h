#pragma once

#include <string_view>
#include <type_traits>
#include "constexpr_map.h"
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
                            const detail::map_c<detail::element_hash<std::string_view, std::string_view>, N>& m)
        : Field(name, type, offset)
        , mMetas(m)
    {
    }

    constexpr int MetaCount() const
    {
        return N;
    }

    const detail::map_c<detail::element_hash<std::string_view, std::string_view>, N>& mMetas;
};

template <>
struct FieldWithMeta<0> : public Field
{
    constexpr FieldWithMeta(const std::string_view name, const std::string_view type, uint32_t offset, std::nullptr_t nptr)
        : Field(name, type, offset)
        , mMetas(nptr)
    {
    }

    constexpr int MetaCount() const
    {
        return 0;
    }
    const detail::map_c<detail::element_hash<std::string_view, std::string_view>, 0> mMetas = nullptr;
};

#define FIELD_INFO(name, Class, map, ...)\
    static struct name##Info\
    {\
        constexpr name##Info() = default;\
        constexpr static const FieldWithMeta<detail::____map_size<decltype(map)>::value> info = \
        FieldWithMeta<detail::____map_size<decltype(map)>::value>(\
            #name, \
            FakeReal::reflection::decay_type_name<decltype(std::declval<Class>().name)>(), \
            offsetof(Class, name), \
            map\
        );\
        decltype(&Class::name) ptr = &Class::name;\
    };

#define STATIC_FIELD_INFO(name, Class, map, ...)\
    static struct name##Info\
    {\
        constexpr name##Info() = default;\
        constexpr static const FieldWithMeta<detail::____map_size<decltype(map)>::value> info = \
        FieldWithMeta<detail::____map_size<decltype(map)>::value>(\
            #name, \
            FakeReal::reflection::decay_type_name<decltype(Class::name)>(), \
            0, \
            map\
        );\
        decltype(&Class::name) ptr = &Class::name;\
    };

#define METHOD_INFO(name, Class, map, ...)\
    static struct name##Info\
    {\
        constexpr name##Info() = default;\
        constexpr static const FieldWithMeta<detail::____map_size<decltype(map)>::value> info = \
        FieldWithMeta<detail::____map_size<decltype(map)>::value>(\
            #name, \
            FakeReal::reflection::decay_type_name<decltype(&Class::name)>(), \
            0, \
            map\
        );\
        decltype(&Class::name) ptr = &Class::name;\
    };

template <typename T>
struct ClassInfo
{
    inline static const constexpr std::string_view GetClassName()
    {
        return FakeReal::reflection::decay_type_name<T>();
    }

    inline static const constexpr auto AllFields() { return std::make_tuple(); }
    inline static const constexpr auto AllStaticFields() { return std::make_tuple(); }
    inline static const constexpr auto AllMethod() { return std::make_tuple(); }
};

template <typename T, std::enable_if_t<!std::is_pointer_v<T>, int> = 0>
inline static const constexpr bool IsAtomic() { return false; }

template <typename T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
inline static const constexpr bool IsAtomic() { return true; }

#define GEN_REFL_BASIC_TYPES_TWO_PARAM(T, NAME)                       \
    template <>                                                       \
    inline const constexpr bool IsAtomic<T>()                         \
    {                                                                 \
        return true;                                                  \
    }                                                                 \
    template <>                                                       \
    struct ClassInfo<T>                                               \
    {                                                                 \
        inline static const constexpr std::string_view GetClassName() \
        {                                                             \
            return #NAME;                                             \
        }                                                             \
        inline static constexpr const auto AllFields()                \
        {                                                             \
            return std::make_tuple();                                 \
        }                                                             \
        inline static constexpr const auto AllStaticFields()          \
        {                                                             \
            return std::make_tuple();                                 \
        }                                                             \
        inline static constexpr const auto AllMethod()                \
        {                                                             \
            return std::make_tuple();                                 \
        }                                                             \
    };

#define GEN_REFL_BASIC_TYPES(T) GEN_REFL_BASIC_TYPES_TWO_PARAM(T, T)

GEN_REFL_BASIC_TYPES(float);

#define HAS_MEMBER(member)                                                          \
    template <typename T>                                                           \
    struct HasMember_##member                                                       \
    {                                                                               \
        template <typename T_T>                                                     \
        static auto check(T_T) -> typename std::decay<decltype(T_T::member)>::type; \
        static void check(...);                                                     \
        using type = decltype(check(std::declval<T>()));                            \
        enum                                                                        \
        {                                                                           \
            value = !std::is_void<type>::value                                      \
        };                                                                          \
    };
/*
    static auto check(T_T) -> typename std::decay<decltype(T_T::member)>::type 是一种类型萃取的方法，可以获得返回类型的decay值.
    所谓的decay值就是去除引用以及const等特性，获得一个被削弱的返回值。
*/

HAS_MEMBER(AllFields)
HAS_MEMBER(AllStaticFields)
HAS_MEMBER(AllMethod)
/*
template <typename T>
struct HasMember_AllFields
{
    template <typename T_T>
    static auto check(T_T) -> typename std ::decay<decltype(T_T ::AllFields)>::type;
    static void check(...);
    using type = decltype(check(std ::declval<T>()));
    enum
    {
        value = !std ::is_void<type>::value
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
inline constexpr void ForeachTuple(Tuple&& tuple, Fn&& fn)
{
    constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
    for_each_impl(std::forward<Tuple>(tuple), std::forward<Fn>(fn), std::make_index_sequence<N>{});
}

template <typename T>
struct FClass
{
    using ClassName = std::decay_t<T>;
    using info      = ClassInfo<ClassName>;

    inline static const constexpr std::string_view GetName()
    {
        return info::GetClassName();
    }

    template <typename Value, typename Fn>
    inline static constexpr void ForeachField(Value&& value, Fn&& func)
    {
        if constexpr (HasMember_AllFields<info>::value)
        {
            __for_each_field_impl<false>(info::AllFields(), value, func);
        }
    }

    template <bool atomic, typename V, typename Fn, typename Tuple>
    inline static constexpr void __for_each_field_impl(Tuple&& tup, V&& value, Fn&& fn)
    {
        ForeachTuple(std::forward<Tuple>(tup),
                     [&value, &fn](auto&& field_schema) {
                         using ClassName_C = std::decay_t<decltype(value.*(field_schema.ptr))>;
                         if constexpr (!IsAtomic<ClassName_C>() && atomic)
                         {
                             /* static_assert(HasMember_AllFields<ClassInfo<ClassName_C>>::value);
                             FClass<ClassName_C>::ForEachFieldAtomic(std::forward<ClassName_C>(value.*(field_schema.ptr)), fn);
                             constexpr int leng = std::tuple_size(ClassInfo<ClassName_C>::all_static_fields());
                             if constexpr (leng > 0)
                             {
                                 static_assert(has_member_all_static_fields<ClassInfo<ClassName_C>>::value);
                                 FClass<ClassName_C>::ForEachStaticFieldAtomic(fn);
                             } */
                         }
                         else
                             fn(value.*(field_schema.ptr), field_schema.info);
                     });
    }

    template <typename Fn>
    inline static constexpr void ForeachStaticField(Fn&& func)
    {
        if constexpr (HasMember_AllStaticFields<info>::value)
        {
            __for_each_static_field_impl<false>(info::AllStaticFields(), func);
        }
    }

    template <bool atomic, typename Fn, typename Tuple>
    inline static constexpr void __for_each_static_field_impl(Tuple&& tup, Fn&& fn)
    {
        ForeachTuple(std::forward<Tuple>(tup),
                     [&fn](auto&& field_schema) {
                         using ClassName_C = std::decay_t<decltype(*(field_schema.ptr))>;
                         if constexpr (!IsAtomic<ClassName_C>() && atomic)
                         {
                             
                         }
                         else
                             fn(*(field_schema.ptr), field_schema.info);
                     });
    }

    template <typename Value, typename Fn>
    inline static constexpr void ForeachMethod(Value&& value, Fn&& func)
    {
        if constexpr (HasMember_AllMethod<info>::value)
        {
            ForeachTuple(info::AllMethod(),
                         [&func, &value](auto&& field_schema) {
                             func(field_schema.ptr, field_schema.info);
                         });
        }
    }
};