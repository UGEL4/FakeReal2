#pragma once

#include "Platform/Config.h"
#include <rapidjson/document.h>
#include <stack>
#include <string>

namespace FakeReal
{
    struct JsonReaderStackItem
    {
        enum State
        {
            BeforeStart, //!< An object/array is in the stack but it is not yet called by StartObject()/StartArray().
            Started,     //!< An object/array is called by StartObject()/StartArray().
            Closed       //!< An array is closed after read all element, but before EndArray().
        };

        JsonReaderStackItem(const rapidjson::Value* value, State state)
            : pValue(value)
            , state(state)
            , index()
        {
        }

        const rapidjson::Value* pValue;
        State state;
        rapidjson::SizeType index; // For array iteration
    };
    class RUNTIME_API JsonReader
    {
    public:
        JsonReader(const char* json);
        ~JsonReader();

        JsonReader& StartObject();
        JsonReader& EndObject();
        JsonReader& Key(const char* name);
        bool HasKey(const char* name) const;

        JsonReader& StartArray(size_t* size = 0);
        JsonReader& EndArray();

        operator bool() const
        {
            return !mError;
        }

        JsonReader& Value(int32_t& val);
        JsonReader& Value(int64_t& val);
        JsonReader& Value(uint32_t& val);
        JsonReader& Value(uint64_t& val);
        JsonReader& Value(float& val);
        JsonReader& Value(double& val);
        JsonReader& Value(bool& val);
        JsonReader& Value(std::string& val);

    private:
        JsonReader(const JsonReader& other);
        JsonReader& operator=(const JsonReader& other);

        void Next();

    private:
        rapidjson::Document* m_pDocument{ nullptr };
        std::stack<JsonReaderStackItem>* m_pStack{ nullptr }; ///< Stack for iterating the DOM
        bool mError{ false };                                 ///< Whether an error has occurred.
    };
} // namespace FakeReal