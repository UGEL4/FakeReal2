#include "Utils/Json/JsonReader.h"
#include "Platform/Config.h"

namespace FakeReal
{
    using namespace rapidjson;
    JsonReader::JsonReader(const char* json)
    {
        m_pDocument = new Document;
        m_pDocument->Parse(json);
        if (m_pDocument->HasParseError())
        {
            mError = true;
        }
        else
        {
            m_pStack = new std::stack<JsonReaderStackItem>;
            m_pStack->push(JsonReaderStackItem(m_pDocument, JsonReaderStackItem::BeforeStart));
        }
    }

    JsonReader::~JsonReader()
    {
        delete m_pStack;
        delete m_pDocument;
    }

    JsonReader& JsonReader::StartObject()
    {
        if (!mError)
        {
            auto& top = m_pStack->top();
            if (top.pValue->IsObject() && top.state == JsonReaderStackItem::BeforeStart)
            {
                top.state = JsonReaderStackItem::Started;
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::EndObject()
    {
        if (!mError)
        {
            auto& top = m_pStack->top();
            if (top.pValue->IsObject() && top.state == JsonReaderStackItem::Started)
            {
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Key(const char* name)
    {
        if (!mError)
        {
            auto& top = m_pStack->top();
            if (top.pValue->IsObject() && top.state == JsonReaderStackItem::Started)
            {
                rapidjson::Value::ConstMemberIterator memberItr = top.pValue->FindMember(name);
                if (memberItr != top.pValue->MemberEnd())
                {
                    m_pStack->push(JsonReaderStackItem(&memberItr->value, JsonReaderStackItem::BeforeStart));
                }
                else
                {
                    mError = true;
                }
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    bool JsonReader::HasKey(const char* name) const
    {
        auto& top = m_pStack->top();
        if (!mError && top.pValue->IsObject() && top.state == JsonReaderStackItem::Started)
        {
            return top.pValue->HasMember(name);
        }
        return false;
    }

    JsonReader& JsonReader::StartArray(size_t* size /*= 0*/)
    {
        if (!mError)
        {
            auto& top = m_pStack->top();
            if (top.pValue->IsArray() && top.state == JsonReaderStackItem::BeforeStart)
            {
                top.state = JsonReaderStackItem::Started;
                if (size)
                {
                    *size = top.pValue->Size();
                }

                if (!top.pValue->Empty())
                {
                    // const rapidjson::Value* value = &top.pValue[top.index];//top.pValue is a pointer, dont do this.
                    const rapidjson::Value* pValue = &(*top.pValue)[top.index]; // top.pValue is a pointer
                    m_pStack->push(JsonReaderStackItem(pValue, JsonReaderStackItem::BeforeStart));
                }
                else
                {
                    top.state = JsonReaderStackItem::Closed;
                }
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::EndArray()
    {
        if (!mError)
        {
            auto& top = m_pStack->top();
            if (top.pValue->IsArray() && top.state == JsonReaderStackItem::Closed)
            {
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(int32_t& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsInt())
            {
                val = value->GetInt();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(int64_t& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsInt64())
            {
                val = value->GetInt64();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(uint32_t& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsUint())
            {
                val = value->GetUint();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(uint64_t& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsUint64())
            {
                val = value->GetUint64();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(float& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsDouble())
            {
                val = static_cast<float>(value->GetDouble());
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(double& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsDouble())
            {
                val = value->GetDouble();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(bool& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsBool())
            {
                val = value->GetBool();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    JsonReader& JsonReader::Value(std::string& val)
    {
        if (!mError)
        {
            auto& value = m_pStack->top().pValue;
            if (value->IsString())
            {
                val = value->GetString();
                Next();
            }
            else
            {
                mError = true;
            }
        }
        return *this;
    }

    void JsonReader::Next()
    {
        if (!mError)
        {
            assert(!m_pStack->empty());
            m_pStack->pop();

            // auto& top = m_pStack->top();//top() maybe invalid.
            if (!m_pStack->empty() && m_pStack->top().pValue->IsArray())
            {
                auto& top = m_pStack->top();
                if (top.state == JsonReaderStackItem::Started)
                { // Otherwise means reading array item pass end
                    if (top.index < top.pValue->Size() - 1)
                    {
                        const rapidjson::Value* pValue = &(*top.pValue)[++top.index];
                        m_pStack->push(JsonReaderStackItem(pValue, JsonReaderStackItem::BeforeStart));
                    }
                    else
                    {
                        top.state = JsonReaderStackItem::Closed;
                    }
                }
                else
                {
                    mError = true;
                }
            }
        }
    }
} // namespace FakeReal