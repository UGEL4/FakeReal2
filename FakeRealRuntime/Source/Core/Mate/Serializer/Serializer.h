#pragma once

#include <string>
#include <rapidjson/writer.h>
#include "Core/Mate/Serializer/JsonWriter.h"
#include "Core/Mate/Serializer/JsonReader.h"
#include "Core/Base/Macro.h"
#include "Core/Mate/Reflection.h"

namespace FakeReal
{
	template<typename...>
	inline constexpr bool always_false = false;

	using namespace rapidjson;
	class Serializer
	{
	public:

		template<typename T>
		static void WritePointer(JsonWriter& writer, T* instance)
		{
			writer.StartObject();
			writer.SetKey("$typename");
			writer.String("*");
			writer.SetKey("$context");
			Serializer::Write(writer, *instance);
			writer.EndObject();
		}

		template<typename T>
		static bool ReadPointer(JsonReader& reader, T*& instance)
		{
			ASSERT(instance == nullptr);
			bool result = true;
			reader.StartObject();
			std::string typeName;
			reader.Key("$typename").Value(typeName);
			if (typeName == "*")
			{
				reader.Key("$context");
				instance = new T;
				Serializer::Read(reader, *instance);
			}
			result = reader.EndObject();
			return result;
		}

		template <typename T>
		static void Write(JsonWriter& writer, const ReflectionPtr<T>& instance)
		{
			T* ptr = static_cast<T*>(instance.operator->());
			writer.StartObject();
			writer.SetKey("$typename");
			writer.String(instance.GetTypeName());
			writer.SetKey("$context");
			Serializer::Write(writer, *ptr);
			writer.EndObject();
		}

		template <typename T>
		static bool Read(JsonReader& reader, ReflectionPtr<T>& instance)
		{
			ASSERT(instance == nullptr);
			bool result = true;
			reader.StartObject();
			std::string typeName;
			reader.Key("$typename").Value(typeName);
			reader.Key("$context");
			instance.GetPtrRef() = new T;
			Serializer::Read(reader, *instance);
			result = reader.EndObject();
			return result;
		}

		template<typename T>
		static void Write(JsonWriter& writer, const T& instance)
		{
			if constexpr (std::is_pointer<T>::value)
			{
				Serializer::WritePointer(writer, instance);
			}
			else
			{
				static_assert(always_false<T>, "Serializer::Write<T> has not been implemented yet!");
			}
		}

		template<typename T>
		static bool Read(JsonReader& reader, T& instance)
		{
			if constexpr (std::is_pointer<T>::value)
			{
				return ReadPointer(reader, instance);
			}
			else
			{
				static_assert(always_false<T>, "Serializer::Read<T> has not been implemented yet!");
				return false;
			}
		}
	};

	template<>
	void Serializer::Write(JsonWriter& writer, const int32_t& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, int32_t& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const int64_t& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, int64_t& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const uint32_t& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, uint32_t& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const uint64_t& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, uint64_t& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const float& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, float& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const double& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, double& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const bool& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, bool& instance);

	template<>
	void Serializer::Write(JsonWriter& writer, const std::string& instance);
	template<>
	bool Serializer::Read(JsonReader& reader, std::string& instance);
}