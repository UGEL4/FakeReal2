#include "FRPch.h"
#include "Core/Mate/Serializer/JsonWriter.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{

	using namespace rapidjson;
	JsonWriter::JsonWriter()
	{
		m_pStream = new StringBuffer;
		m_pWriter = new PrettyWriter<StringBuffer>(*m_pStream);
	}

	JsonWriter::JsonWriter(size_t bufferSize)
	{
		m_pStream = new StringBuffer(0, bufferSize);
		m_pWriter = new PrettyWriter<StringBuffer>(*m_pStream);
	}

	JsonWriter::~JsonWriter()
	{
		delete m_pWriter;
		delete m_pStream;
	}

	std::string JsonWriter::GetString()
	{
		return m_pStream->GetString();
	}

	JsonWriter& JsonWriter::StartObject()
	{
		m_pWriter->StartObject();
		return *this;
	}

	JsonWriter& JsonWriter::SetKey(const char* name)
	{
		m_pWriter->String(name, static_cast<SizeType>(strlen(name)));
		return *this;
	}

	bool JsonWriter::HasKey(const char* name) const
	{
		ASSERT(false);
		return false;
	}

	JsonWriter& JsonWriter::EndObject()
	{
		m_pWriter->EndObject();
		return *this;
	}

	JsonWriter& JsonWriter::StartArray()
	{
		m_pWriter->StartArray();
		return *this;
	}

	JsonWriter& JsonWriter::EndArray()
	{
		m_pWriter->EndArray();
		return *this;
	}

	JsonWriter& JsonWriter::SetInt(const char* key, int32_t val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Int(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetInt64(const char* key, int64_t val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Int64(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetUInt(const char* key, uint32_t val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Uint(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetUInt64(const char* key, uint64_t val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Uint64(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetDouble(const char* key, double val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Double(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetBool(const char* key, bool val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->Bool(val);
		return *this;
	}

	JsonWriter& JsonWriter::SetString(const char* key, const std::string& val)
	{
		m_pWriter->Key(key, static_cast<SizeType>(strlen(key)));
		m_pWriter->String(val.c_str());
		return *this;
	}

	JsonWriter& JsonWriter::Int(int32_t val)
	{
		m_pWriter->Int(val);
		return *this;
	}

	JsonWriter& JsonWriter::Int64(int64_t val)
	{
		m_pWriter->Int64(val);
		return *this;
	}

	JsonWriter& JsonWriter::UInt(uint32_t val)
	{
		m_pWriter->Uint(val);
		return *this;
	}

	JsonWriter& JsonWriter::UInt64(uint64_t val)
	{
		m_pWriter->Uint64(val);
		return *this;
	}

	JsonWriter& JsonWriter::Double(double val)
	{
		m_pWriter->Double(val);
		return *this;
	}

	JsonWriter& JsonWriter::Bool(bool val)
	{
		m_pWriter->Bool(val);
		return *this;
	}

	JsonWriter& JsonWriter::String(const std::string& val)
	{
		m_pWriter->String(val.c_str());
		return *this;
	}
}