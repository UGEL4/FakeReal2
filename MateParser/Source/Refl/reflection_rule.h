#pragma once

#include <string_view>

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
    constexpr int MetaCount() const
    {
        return N;
    }
};