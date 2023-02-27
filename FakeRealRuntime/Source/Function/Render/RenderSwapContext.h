#pragma once

#include <deque>
#include <optional>
#include "Function/Render/RenderObject.h"

namespace FakeReal
{
	struct GameObjectResourceDesc
	{
		std::deque<GameObjectDesc> mGameObjectsDesc;
		void Add(const GameObjectDesc& desc);
		void Pop();
		bool Empty() const;
		GameObjectDesc& GetNextGameObject();
	};

	struct RenderSwapData
	{
		std::optional<GameObjectResourceDesc> mGameObjectResourecDesc;

		void AddDirtyGameObject(GameObjectDesc&& gameObjectDesc);
	};

	enum SwapDataType : uint8_t
	{
		SwapDataTypeLogic = 0,
		SwapDataTypeRender,
		SwapDataTypeCount,
	};

	class RenderSwapContext
	{
	public:
		RenderSwapData& GetLogicSwapData();
		RenderSwapData& GetRenderSwapData();
		void SwapLogicRenderData();
		void ResetGameObjectResourceDescData();
	private:
		bool ReadyToSwap() const;
		void Swap();

	private:
		uint8_t mLogicSwapDataIndex { SwapDataTypeLogic };
		uint8_t mRenderSwapDataIndex { SwapDataTypeRender };
		RenderSwapData mRenderSwapData[SwapDataTypeCount];
	};
}