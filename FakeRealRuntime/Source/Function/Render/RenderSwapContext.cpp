#include "FRPch.h"
#include "Function/Render/RenderSwapContext.h"

namespace FakeReal
{
	void GameObjectResourceDesc::Add(const GameObjectDesc& desc)
	{
		mGameObjectsDesc.emplace_back(desc);
	}

	void GameObjectResourceDesc::Pop()
	{
		mGameObjectsDesc.pop_front();
	}

	bool GameObjectResourceDesc::Empty() const
	{
		return mGameObjectsDesc.empty();
	}

	GameObjectDesc& GameObjectResourceDesc::GetNextGameObject()
	{
		return mGameObjectsDesc.front();
	}

	/////////////////////////////////////////////////////
	void RenderSwapData::AddDirtyGameObject(GameObjectDesc&& gameObjectDesc)
	{
		if (mGameObjectResourecDesc.has_value())
		{
			mGameObjectResourecDesc->Add(gameObjectDesc);
		}
		else
		{
			GameObjectResourceDesc tmp;
			tmp.Add(gameObjectDesc);
			mGameObjectResourecDesc = tmp;
		}
	}
	/////////////////////////////////////////////////////
	RenderSwapData& RenderSwapContext::GetLogicSwapData()
	{
		return mRenderSwapData[mLogicSwapDataIndex];
	}

	RenderSwapData& RenderSwapContext::GetRenderSwapData()
	{
		return mRenderSwapData[mRenderSwapDataIndex];
	}

	void RenderSwapContext::SwapLogicRenderData()
	{
		if (ReadyToSwap())
		{
			Swap();
		}
	}

	void RenderSwapContext::ResetGameObjectResourceDescData()
	{
		mRenderSwapData[mRenderSwapDataIndex].mGameObjectResourecDesc.reset();
	}

	bool RenderSwapContext::ReadyToSwap() const
	{
		return !mRenderSwapData[mRenderSwapDataIndex].mGameObjectResourecDesc.has_value();
	}

	void RenderSwapContext::Swap()
	{
		ResetGameObjectResourceDescData();
		std::swap(mLogicSwapDataIndex, mRenderSwapDataIndex);
	}

}