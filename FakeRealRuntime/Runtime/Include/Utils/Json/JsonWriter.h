#pragma once

#include <string>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "Platform/Config.h"

namespace FakeReal
{
    class RUNTIME_API JsonWriter
    {
    public:
        JsonWriter();
        JsonWriter(size_t bufferSize);
        ~JsonWriter();

        // Get json string
        std::string GetString();

        JsonWriter& StartObject();
        JsonWriter& SetKey(const char* name);
        bool HasKey(const char* name) const;
        JsonWriter& EndObject();

        JsonWriter& StartArray();
        JsonWriter& EndArray();

        JsonWriter& SetInt(const char* key, int32_t val);
        JsonWriter& SetInt64(const char* key, int64_t val);
        JsonWriter& SetUInt(const char* key, uint32_t val);
        JsonWriter& SetUInt64(const char* key, uint64_t val);
        JsonWriter& SetDouble(const char* key, double val);
        JsonWriter& SetBool(const char* key, bool val);
        JsonWriter& SetString(const char* key, const std::string& val);

        JsonWriter& Int(int32_t val);
        JsonWriter& Int64(int64_t val);
        JsonWriter& UInt(uint32_t val);
        JsonWriter& UInt64(uint64_t val);
        JsonWriter& Double(double val);
        JsonWriter& Bool(bool val);
        JsonWriter& String(const std::string& val);

    private:
        rapidjson::StringBuffer* m_pStream;
        rapidjson::PrettyWriter<rapidjson::StringBuffer>* m_pWriter;
    };
} // namespace FakeReal