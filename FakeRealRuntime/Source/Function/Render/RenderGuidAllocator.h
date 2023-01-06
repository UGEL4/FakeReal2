#pragma once

#include <unordered_map>
namespace FakeReal
{
	static const size_t s_Invalid_Guid = 0;
	template <typename T>
	class GuidAllocator
	{
	public:

		bool IsValidGuid(size_t guid) const { return guid != s_Invalid_Guid; }

		size_t AllocGuid(const T& e)
		{
			auto itr = mElementsGuidMap.find(e);
			if (itr != mElementsGuidMap.end())
			{
				return itr->second;
			}

			for (size_t i = 0; i < mElementsGuidMap.size() + 1; i++)
			{
				size_t guid = i + 1;
				if (mGuidElementsMap.find(guid) == mGuidElementsMap.end())
				{
					mElementsGuidMap.insert(std::make_pair(e, guid));
					mGuidElementsMap.insert(std::make_pair(guid, e));
					return guid;
				}
			}
			return s_Invalid_Guid;
		}

		bool GetGuidRelatedElement(size_t guid, T& e)
		{
			auto itr = mGuidElementsMap.find(guid);
			if (itr != mGuidElementsMap.end())
			{
				e = itr->second;
				return true;
			}
			return false;
		}

		bool GetElementGuid(const T& e, size_t& guid)
		{
			auto itr = mElementsGuidMap.find(e);
			if (itr != mElementsGuidMap.end())
			{
				guid = itr->second;
				return true;
			}
			return false;
		}

		bool HasElement(const T& e) { return mElementsGuidMap.find(e) != mElementsGuidMap.end(); }

		void FreeGuid(size_t guid)
		{
			auto itr = mGuidElementsMap.find(guid);
			if (itr != mGuidElementsMap.end())
			{
				const auto& e = itr->second;
				mElementsGuidMap.erase(e);
				mGuidElementsMap.erase(guid);
			}
		}

		void FreeElement(const T& e)
		{
			auto itr = mElementsGuidMap.find(e);
			if (itr != mElementsGuidMap.end())
			{
				const auto& guid = itr->second;
				mGuidElementsMap.erase(guid);
				mElementsGuidMap.erase(e);
			}
		}

		std::vector<size_t> GetAllocatedGuids() const
		{
			std::vector<size_t> guids(mGuidElementsMap.size());
			size_t i = 0;
			for (const auto& itr : mGuidElementsMap)
			{
				guids[i++] = itr.first;
			}
			return guids;
		}

		void Clear()
		{
			mElementsGuidMap.clear();
			mGuidElementsMap.clear();
		}
	private:
		std::unordered_map<T, size_t> mElementsGuidMap;
		std::unordered_map<size_t, T> mGuidElementsMap;
	};
}