#pragma once

#include <vector>
#include "Engine/Vertex.h"
#include "Engine/Mesh.h"
#define	MAX_QUAD_LEVEL_COUNT	13




namespace Maple 
{

	struct VertNode;
	struct QuadNode;
	struct QuadLeaf;

	class Camera;
	class QuadCollapseMesh  : public Mesh
	{
	public:
		QuadCollapseMesh();
		~QuadCollapseMesh();

		auto build(const std::vector<Vertex>& vertices, uint32_t width, uint32_t height) -> bool;

	

		auto update(Camera* camera) -> void;
		auto getMaxLevelLength() const->int32_t;
		auto getType()  -> MeshType override { return TERRAIN; }

		inline auto setLod(bool lod) {
			this->lod = lod;
		}

		inline auto isLod() const { return lod; }
		inline auto isCalInVertex() const { return calInVertex; }



		std::shared_ptr<Texture> heightMap;
	private:
		bool lod = false;
		bool calInVertex = false;

		std::vector<uint32_t> indices;
		std::vector<uint32_t> lodIndices;

		std::vector<Vertex> vertices;
		std::vector<Vertex> activeVertices;

		int32_t	maxLevelVerticesLength = 0;
		uint32_t maxLevel = 0;
		float vertNodesActiveDistance[MAX_QUAD_LEVEL_COUNT] = {};
		float quadNodesCullRadius[MAX_QUAD_LEVEL_COUNT] = {};
		int32_t	  vertNodesLevelOffset[MAX_QUAD_LEVEL_COUNT] = {};

		std::vector<VertNode> vertNodePool;
		std::vector<QuadNode> quadNodePool;
		std::vector<QuadLeaf> quadLeafPool;
		uint32_t quadNodePoolAllocated = 0;
		uint32_t quadLeafPoolAllocated = 0;

		// root nodes
		QuadNode* rootQuadNode = nullptr;
		VertNode* rootVertNodes[4] = {};
		uint32_t updateFrame = 0;


		auto buildVertNodes() -> void;
		auto recursiveBuildQuadNodes(uint32_t level, int32_t x0, int32_t y0, int32_t step) ->QuadNode *;
		auto collapseQuad(QuadNode * quadNode, int32_t x0, int32_t y0, int32_t step) -> void;

		auto getVertNode(uint32_t level, int32_t x, int32_t y, bool init_mode) -> VertNode*;
		auto allocQuadNode() ->QuadNode*;
		auto allocQuadLeaf() ->QuadLeaf*;

		auto recursiveUpdateVertNode(Camera* camera, VertNode* vertNode) -> void;
		auto quadNodeSetBoundary(Camera* camera, QuadNode* quadNode) -> void;
		auto recursiveSetActiveMesh(QuadNode* quadNode) -> void;
		auto addActiveVertNode(VertNode* vertNode)-> void;

	};
};