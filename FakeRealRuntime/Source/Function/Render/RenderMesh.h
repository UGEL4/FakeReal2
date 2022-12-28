#pragma once
#include <vulkan/vulkan.h>
#include <vector>
namespace FakeReal
{
	struct MeshVertex
	{
		struct VertexPosition
		{
			float x, y, z;
		};
		struct VertexNormal
		{
			float x, y, z;
		};
		struct VertexTexcoord
		{
			float x, y;
		};

		VertexPosition pos;
		VertexNormal normal;
		VertexTexcoord uv;

		static std::vector<VkVertexInputBindingDescription> GetVulkanVertexBindingDescription()
		{
			std::vector<VkVertexInputBindingDescription> bindingDesc(1);
			bindingDesc[0].binding		= 0;
			bindingDesc[0].stride		= sizeof(MeshVertex);
			bindingDesc[0].inputRate	= VK_VERTEX_INPUT_RATE_VERTEX;

			return bindingDesc;
		}

		static std::vector<VkVertexInputAttributeDescription> GetVulkanVertexInputAttributeDescriptions()
		{
			std::vector<VkVertexInputAttributeDescription> descs(3);
			descs[0].location	= 0;
			descs[0].binding	= 0;
			descs[0].offset		= 0;
			descs[0].format		= VK_FORMAT_R32G32B32_SFLOAT;

			descs[1].location	= 1;
			descs[1].binding	= 0;
			descs[1].offset		= sizeof(VertexPosition);
			descs[1].format		= VK_FORMAT_R32G32B32_SFLOAT;

			descs[2].location	= 2;
			descs[2].binding	= 0;
			descs[2].offset		= sizeof(VertexPosition) + sizeof(VertexNormal);
			descs[2].format		= VK_FORMAT_R32G32_SFLOAT;

			return descs;
		}
	};
}