
#include "QuadCollapseMesh.h"
#include "Others/Console.h"
#include "Engine/Camera.h"
#include <imgui.h>

namespace Maple 
{
	static const float	ACTIVE_SCALE = 16.0f;
	static const int	MAX_LENGTH = 4097;

	bool isPowerOf2(int32_t n) {
		return n && !(n & (n - 1));
	}

	enum TriangulationMode {
		TM_NW_SE,
		TM_SW_NE,
	};

	enum NodeState
	{
		NS_ACTIVE,
		NS_BOUNDARY
	};


	struct VertNode {
		struct {
			uint32_t	activeFrame : 24;
			uint32_t	state : 1;
			uint32_t	level : 4;
			uint32_t	adjcentQuadsCount : 3;
		};
		Vertex originalVertex;
		float interpolationFactor;	// position interpolation
		Vertex interpolatedVertex;

		VertNode* parent = nullptr;
		VertNode* firstChild = nullptr;
		VertNode* nextSibling = nullptr;
		QuadNode* adjcentQuads[4] = {};

		auto addChild(VertNode* child);
		auto addAdjcentQuad(QuadNode* quadNode);
		auto hasChild(VertNode* testNode) const -> bool;
	};

	auto VertNode::addChild(VertNode* child)
	{
		if (child->parent == this) {
			return; 
		}

		if (child->level != level + 1) {
			return;
		}

		child->parent = this;

		if (child->nextSibling) {
			return;
		}

		if (firstChild) {
			child->nextSibling = firstChild;
			firstChild = child;
		}
		else {
			firstChild = child;
		}if (child->parent == this) {
			return; 
		}

		if (child->level != level + 1) {
			return;
		}

		child->parent = this;

		if (child->nextSibling) {
			return;
		}

		if (firstChild) {
			child->nextSibling = firstChild;
			firstChild = child;
		}
		else {
			firstChild = child;
		}
	}

	auto VertNode::addAdjcentQuad(QuadNode* quadNode)
	{
		for (int i = 0; i < adjcentQuadsCount; i++) {
			if (adjcentQuads[i] == quadNode) {
				return;
			}
		}
		adjcentQuads[adjcentQuadsCount++] = quadNode;
	}

	auto VertNode::hasChild(VertNode* testNode) const -> bool
	{
		VertNode* child = firstChild;
		while (child) {
			if (child == testNode) {
				return true;
			}
			child = child->nextSibling;
		}
		return false;
	}


	struct QuadNode {
		struct {
			uint32_t activeFrame : 26;
			uint32_t state : 1;
			uint32_t level : 4;
			uint32_t triangulationMode : 1;
		};
		QuadNode* parent = nullptr; // null if it is root
		VertNode* cornerVertNodes[4] = {};
		VertNode* centerVertNode = nullptr;
		QuadNode* children[4] = {};

		auto isDiagonalConnected(const VertNode* vn1, const VertNode* vn2) const -> bool;
	};

	auto QuadNode::isDiagonalConnected(const VertNode* vn1, const VertNode* vn2) const -> bool
	{
		if (triangulationMode == TM_NW_SE)
		{
			return ((vn1 == cornerVertNodes[1] && vn2 == cornerVertNodes[3]))
				|| ((vn1 == cornerVertNodes[3] && vn2 == cornerVertNodes[1]));
		}
		else // TM_SW_NE
		{
			return ((vn1 == cornerVertNodes[0] && vn2 == cornerVertNodes[2]))
				|| ((vn1 == cornerVertNodes[2] && vn2 == cornerVertNodes[0]));
		}
	}

	struct QuadLeaf {
		struct {
			uint32_t				activeFrame : 26;
			uint32_t				state : 1;
			uint32_t				level : 4;
			uint32_t				triangleMode : 1;
		};
		QuadNode* parent = nullptr;
		VertNode* cornerVertNodes[4] = {};
	};



	QuadCollapseMesh::QuadCollapseMesh()
	{
	}

	QuadCollapseMesh::~QuadCollapseMesh()
	{
	}

	auto QuadCollapseMesh::build(const std::vector<Vertex>& vertices, uint32_t width, uint32_t height) -> bool
	{
		this->vertices = vertices;

		for (auto i = 0; i < vertices.size(); i++)
		{
			lodIndices.emplace_back(i);
		}

		uint32_t numIndices = (width - 1) * (height - 1) * 6;
		indices.resize(numIndices);
		size = numIndices;
		int32_t indicesCount = 0;
		for (int32_t x = 0; x < width - 1; ++x)
		{
			for (int32_t y = 0; y < height - 1; ++y)
			{
				if ((uint32_t)indicesCount < numIndices - 6)
				{
					int32_t a = (x * (width)) + y;
					int32_t b = ((x + 1) * (width)) + y;
					int32_t c = ((x + 1) * (width)) + (y + 1);
					int32_t d = (x * (width)) + (y + 1);

					indices[indicesCount++] = c;
					indices[indicesCount++] = b;
					indices[indicesCount++] = a;

					indices[indicesCount++] = a;
					indices[indicesCount++] = d;
					indices[indicesCount++] = c;
				}
			}
		}


		indexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());
		vertexBuffer = std::make_shared<VertexBuffer>();

		maxLevelVerticesLength = width;
		if (maxLevelVerticesLength < 2 /*|| !isPowerOf2(maxLevelVerticesLength - 1)*/) {
 			LOGE("wrong length");
			return false;
		}

		if (maxLevelVerticesLength > MAX_LENGTH) {
			LOGE("height field too large");
			return false;
		}

		int32_t temp = maxLevelVerticesLength - 1;
		int32_t levelCount = 0;// compute how many LODs for a model.
		while (temp) {
			levelCount++;
			temp >>= 1;
		}

		if (levelCount > MAX_QUAD_LEVEL_COUNT) {
			LOGE("height map is too large");
			return false;
		}

		maxLevel = levelCount - 1; // level range: 0 ~ maxLevel

		int32_t quadCountPerEdge = 1;
		int32_t vertCountPerEdge = 2;

		auto quadLeafPoolSize = 0;
		auto quadNodePoolSize = 0;
		auto vertNodePoolSize = 0;

		for (int i = 0; i < levelCount; ++i) {
			float quadSize = (float)((maxLevelVerticesLength - 1) >> i);
			float quadHalfSize = quadSize * 0.5f;
			float quadNodeCullRadius = std::sqrtf(quadHalfSize * quadSize * 2.0f);

			quadNodesCullRadius[i] = quadNodeCullRadius;
			vertNodesActiveDistance[i] = quadNodeCullRadius * ACTIVE_SCALE;

			if (i == levelCount - 1) { // last level
				//the leaf pool
				quadLeafPoolSize = quadCountPerEdge * quadCountPerEdge;
			}
			else {
				quadNodePoolSize += quadCountPerEdge * quadCountPerEdge;
			}

			vertNodesLevelOffset[i] = vertNodePoolSize;
			vertNodePoolSize += (vertCountPerEdge * vertCountPerEdge);

			quadCountPerEdge <<= 1;
			vertCountPerEdge = vertCountPerEdge + vertCountPerEdge - 1;
		}


		vertNodePool.resize(vertNodePoolSize);
		quadNodePool.resize(quadNodePoolSize);
		quadLeafPool.resize(quadLeafPoolSize);


		buildVertNodes();

		rootQuadNode = recursiveBuildQuadNodes(0, 0, 0, maxLevelVerticesLength - 1);

		return true;
	}


	auto QuadCollapseMesh::update(Camera* camera) -> void
	{
		updateFrame++;

		if (lod)
		{
			for (auto i = 0; i < 4; ++i) {
				rootVertNodes[i]->interpolatedVertex = rootVertNodes[i]->originalVertex;
				recursiveUpdateVertNode(camera, rootVertNodes[i]);
			}
			activeVertices.clear();
			recursiveSetActiveMesh(rootQuadNode);
			if (activeVertices.size() != 0)
			{
				vertexBuffer->setData(sizeof(Vertex) * activeVertices.size(), activeVertices.data());
				size = activeVertices.size();
				if(indexBuffer->getSize() != sizeof(uint32_t) * lodIndices.size())
					indexBuffer->setData(sizeof(uint32_t) * lodIndices.size(), lodIndices.data());
			}
		}
		else 
		{
			if (sizeof(Vertex) * vertices.size() != vertexBuffer->getSize()) 
			{
				vertexBuffer->setData(sizeof(Vertex) * vertices.size(), vertices.data());
				size = indices.size();
				if (indexBuffer->getSize() != sizeof(uint32_t) * indices.size())
					indexBuffer->setData(sizeof(uint32_t) * size, indices.data());
			}
		}
	}

	auto QuadCollapseMesh::getMaxLevelLength() const -> int32_t
	{
		return maxLevelVerticesLength;
	}



	auto QuadCollapseMesh::buildVertNodes() -> void
	{
		uint32_t level = 0;
		int32_t step = maxLevelVerticesLength - 1;//width - 1;
		//step 2048 -> 1024 -> 512 -> 256 -> 128 -> 64 -> 32 -> 16 -> 8 -> 4 -> 2 -> 1
		while (step > 0) {
			for (int32_t y = 0; y < maxLevelVerticesLength; y += step) {
				for (int32_t x = 0; x < maxLevelVerticesLength; x += step) {
					auto vertNode = getVertNode(level, x, y, true);

					vertNode->activeFrame = 0;
					vertNode->state = 0;
					vertNode->level = level;
					vertNode->adjcentQuadsCount = 0;
					vertNode->originalVertex = vertices[y * maxLevelVerticesLength + x];
					vertNode->interpolationFactor = 1.0f;
					vertNode->interpolatedVertex = vertNode->originalVertex;
					vertNode->parent = nullptr;
					vertNode->firstChild = nullptr;
					vertNode->nextSibling = nullptr;
					memset(vertNode->adjcentQuads, 0, sizeof(vertNode->adjcentQuads));
				}
			}

			step >>= 1;
			level++;
		}
		//for the root, the quad nodes are 
		/*
		* { 0,		  0 }	{ 0,	    width - 1 }
		* { width - 1,0 } , { width - 1,width - 1 }
		*/
		rootVertNodes[0] = getVertNode(0, 0, 0, false);
		rootVertNodes[1] = getVertNode(0, maxLevelVerticesLength - 1, 0, false);
		rootVertNodes[2] = getVertNode(0, maxLevelVerticesLength - 1, maxLevelVerticesLength - 1, false);
		rootVertNodes[3] = getVertNode(0, 0, maxLevelVerticesLength - 1, false);
	}

	auto QuadCollapseMesh::recursiveBuildQuadNodes(uint32_t level, int32_t x0, int32_t y0, int32_t step) -> QuadNode*
	{
		if (step == 1) { // leaf node
			auto result = allocQuadLeaf();

			result->activeFrame = 0;
			result->state = 0;
			result->level = level;
			result->triangleMode = TM_NW_SE;
			result->parent = nullptr;
			result->cornerVertNodes[0] = getVertNode(level, x0, y0, false);
			result->cornerVertNodes[1] = getVertNode(level, x0 + step, y0, false);
			result->cornerVertNodes[2] = getVertNode(level, x0 + step, y0 + step, false);
			result->cornerVertNodes[3] = getVertNode(level, x0, y0 + step, false);

			for (int32_t i = 0; i < 4; ++i) {
				auto nodes = result->cornerVertNodes[i];
				if (nodes->level == level) {
					nodes->addAdjcentQuad((QuadNode*)result);
				}
			}

			return (QuadNode*)result;
		}
		else {
			auto result = allocQuadNode();

			result->activeFrame = 0;
			result->state = 0;
			result->level = level;
			result->parent = nullptr;

			int32_t halfStep = step >> 1;
			result->centerVertNode =	 getVertNode(level + 1, x0 + halfStep, y0 + halfStep, false);
			/*
			* { x0,		  y0 }	 { x0 + step,		 y0 }
			* { x0, y0 + step} , { x0 + step, y0 + step }
			*/
			result->cornerVertNodes[0] = getVertNode(level, x0, y0, false);
			result->cornerVertNodes[1] = getVertNode(level, x0 + step, y0, false);
			result->cornerVertNodes[2] = getVertNode(level, x0 + step, y0 + step, false);
			result->cornerVertNodes[3] = getVertNode(level, x0, y0 + step, false);

			for (int32_t i = 0; i < 4; ++i) {
				auto corneVertNodes = result->cornerVertNodes[i];
				corneVertNodes->addAdjcentQuad(result);
			}

			uint32_t nextLevel = level + 1;

			result->children[0] = recursiveBuildQuadNodes(nextLevel, x0, y0, halfStep);
			result->children[1] = recursiveBuildQuadNodes(nextLevel, x0 + halfStep, y0, halfStep);
			result->children[2] = recursiveBuildQuadNodes(nextLevel, x0 + halfStep, y0 + halfStep, halfStep);
			result->children[3] = recursiveBuildQuadNodes(nextLevel, x0, y0 + halfStep, halfStep);

			for (int i = 0; i < 4; ++i) {
				result->children[i]->parent = result;
			}

			collapseQuad(result, x0, y0, step);

			return result;
		}
	}

	auto QuadCollapseMesh::collapseQuad(QuadNode* quadNode, int32_t x0, int32_t y0, int32_t step) -> void
	{
		auto vertNodeChildBt = quadNode->children[0]->cornerVertNodes[1];
		auto vertNodeChildRt = quadNode->children[1]->cornerVertNodes[2];
		auto vertNodeChildTp = quadNode->children[2]->cornerVertNodes[3];
		auto vertNodeChildLf = quadNode->children[3]->cornerVertNodes[0];
		auto vertNodeChildCt = quadNode->children[0]->cornerVertNodes[2];
		auto vertNodeChildSw = quadNode->children[0]->cornerVertNodes[0];
		auto vertNodeChildSe = quadNode->children[1]->cornerVertNodes[1];
		auto vertNodeChildNe = quadNode->children[2]->cornerVertNodes[2];
		auto vertNodeChildNw = quadNode->children[3]->cornerVertNodes[3];
		auto vertNodeParentSw = quadNode->cornerVertNodes[0];
		auto vertNodeParentSe = quadNode->cornerVertNodes[1];
		auto vertNodeParentNe = quadNode->cornerVertNodes[2];
		auto vertNodeParentNw = quadNode->cornerVertNodes[3];

		vertNodeParentSw->addChild(vertNodeChildSw);
		vertNodeParentSe->addChild(vertNodeChildSe);
		vertNodeParentNe->addChild(vertNodeChildNe);
		vertNodeParentNw->addChild(vertNodeChildNw);

		if (vertNodeChildCt->parent) {
			LOGE("vert_node_child_ct->parent is not null\n");
			return;
		}

		if (!vertNodeChildBt->parent) {
			auto delta1 = vertNodeChildBt->originalVertex - vertNodeParentSe->originalVertex;
			auto delta2 = vertNodeChildBt->originalVertex - vertNodeParentSw->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentSe->addChild(vertNodeChildBt);
			}
			else {
				vertNodeParentSw->addChild(vertNodeChildBt);
			}
		}

		if (!vertNodeChildRt->parent) {
			auto delta1 = vertNodeChildRt->originalVertex - vertNodeParentNe->originalVertex;
			auto delta2 = vertNodeChildRt->originalVertex - vertNodeParentSe->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentNe->addChild(vertNodeChildRt);
			}
			else {
				vertNodeParentSe->addChild(vertNodeChildRt);
			}
		}

		if (!vertNodeChildTp->parent) {
			auto delta1 = vertNodeChildTp->originalVertex - vertNodeParentNe->originalVertex;
			auto delta2 = vertNodeChildTp->originalVertex - vertNodeParentNw->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentNe->addChild(vertNodeChildTp);
			}
			else {
				vertNodeParentNw->addChild(vertNodeChildTp);
			}
		}

		if (!vertNodeChildLf->parent) {
			auto delta1 = vertNodeChildLf->originalVertex - vertNodeParentNw->originalVertex;
			auto delta2 = vertNodeChildLf->originalVertex - vertNodeParentSw->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentNw->addChild(vertNodeChildLf);
			}
			else {
				vertNodeParentSw->addChild(vertNodeChildLf);
			}
		}

		if (vertNodeParentSw->hasChild(vertNodeChildLf)
			&& vertNodeParentSw->hasChild(vertNodeChildBt)
			&& vertNodeParentNe->hasChild(vertNodeChildRt)
			&& vertNodeParentNe->hasChild(vertNodeChildTp))
		{
			auto delta1 = vertNodeChildCt->originalVertex - vertNodeParentSw->originalVertex;
			auto delta2 = vertNodeChildCt->originalVertex - vertNodeParentNe->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentSw->addChild(vertNodeChildCt);
			}
			else {
				vertNodeParentNe->addChild(vertNodeChildCt);
			}
		}
		else if (
			vertNodeParentSe->hasChild(vertNodeChildBt)
			&& vertNodeParentSe->hasChild(vertNodeChildRt)
			&& vertNodeParentNw->hasChild(vertNodeChildTp)
			&& vertNodeParentNw->hasChild(vertNodeChildLf))
		{
			auto delta1 = vertNodeChildCt->originalVertex - vertNodeParentSe->originalVertex;
			auto delta2 = vertNodeChildCt->originalVertex - vertNodeParentNw->originalVertex;

			if (glm::length(delta1.pos) < glm::length(delta2.pos)) {
				vertNodeParentSe->addChild(vertNodeChildCt);
			}
			else {
				vertNodeParentNw->addChild(vertNodeChildCt);
			}
		}
		else if (
			vertNodeParentSw->hasChild(vertNodeChildLf)
			&& vertNodeParentSw->hasChild(vertNodeChildBt))
		{
			vertNodeParentSw->addChild(vertNodeChildCt);
		}
		else if (
			vertNodeParentSe->hasChild(vertNodeChildBt)
			&& vertNodeParentSe->hasChild(vertNodeChildRt))
		{
			vertNodeParentSe->addChild(vertNodeChildCt);
		}
		else if (
			vertNodeParentNe->hasChild(vertNodeChildRt)
			&& vertNodeParentNe->hasChild(vertNodeChildTp))
		{
			vertNodeParentNe->addChild(vertNodeChildCt);
		}
		else if (
			vertNodeParentNw->hasChild(vertNodeChildTp)
			&& vertNodeParentNw->hasChild(vertNodeChildLf))
		{
			vertNodeParentNw->addChild(vertNodeChildCt);
		}
		else {
			if (quadNode->children[0]->isDiagonalConnected(vertNodeChildCt, vertNodeChildSw)) {
				vertNodeParentSw->addChild(vertNodeChildCt);
			}
			else if (quadNode->children[1]->isDiagonalConnected(vertNodeChildCt, vertNodeChildSe)) {
				vertNodeParentSe->addChild(vertNodeChildCt);
			}
			else if (quadNode->children[2]->isDiagonalConnected(vertNodeChildCt, vertNodeChildNe)) {
				vertNodeParentNe->addChild(vertNodeChildCt);
			}
			else if (quadNode->children[3]->isDiagonalConnected(vertNodeChildCt, vertNodeChildNw)) {
				vertNodeParentNw->addChild(vertNodeChildCt);
			}
			else {
				float dist[4];

				VertNode * parents[4] = { vertNodeParentSw, vertNodeParentSe, vertNodeParentNe, vertNodeParentNw };

				for (int i = 0; i < 4; ++i) {
					auto delta = vertNodeChildCt->originalVertex - parents[i]->originalVertex;
					dist[i] = glm::length(delta.pos);
				}

				float minDist = dist[0];
				int32_t minParentIdx = 0;
				for (int32_t i = 1; i < 4; ++i) {
					if (dist[i] < minDist) {
						minParentIdx = i;
						minDist = dist[i];
					}
				}

				parents[minParentIdx]->addChild(vertNodeChildCt);
			}
		}

		// determine triangle layout of current quadnode

		if (vertNodeChildCt->parent == vertNodeParentSw) {
			if (vertNodeParentNe->hasChild(vertNodeChildTp) || vertNodeParentNe->hasChild(vertNodeChildRt)) {
				quadNode->triangulationMode = TM_SW_NE;
			}
			else {
				if (quadNode->children[2]->isDiagonalConnected(vertNodeChildCt, vertNodeChildNe)) {
					quadNode->triangulationMode = TM_SW_NE;
				}
				else {
					quadNode->triangulationMode = TM_NW_SE;
				}
			}
		}
		else if (vertNodeChildCt->parent == vertNodeParentSe) {
			if (vertNodeParentNw->hasChild(vertNodeChildTp) || vertNodeParentNw->hasChild(vertNodeChildLf)) {
				quadNode->triangulationMode = TM_NW_SE;
			}
			else {
				if (quadNode->children[3]->isDiagonalConnected(vertNodeChildCt, vertNodeChildNw)) {
					quadNode->triangulationMode = TM_NW_SE;
				}
				else {
					quadNode->triangulationMode = TM_SW_NE;
				}
			}
		}
		else if (vertNodeChildCt->parent == vertNodeParentNe) {
			if (vertNodeParentSw->hasChild(vertNodeChildBt) || vertNodeParentSw->hasChild(vertNodeChildLf)) {
				quadNode->triangulationMode = TM_SW_NE;
			}
			else {
				if (quadNode->children[0]->isDiagonalConnected(vertNodeChildCt, vertNodeChildSw)) {
					quadNode->triangulationMode = TM_SW_NE;
				}
				else {
					quadNode->triangulationMode = TM_NW_SE;
				}
			}
		}
		else // vert_node_child_ct->parent == vertNodeParentNW
		{
			if (vertNodeParentSe->hasChild(vertNodeChildBt) || vertNodeParentSe->hasChild(vertNodeChildRt)) {
				quadNode->triangulationMode = TM_NW_SE;
			}
			else {
				if (quadNode->children[1]->isDiagonalConnected(vertNodeChildCt, vertNodeChildSe)) {
					quadNode->triangulationMode = TM_NW_SE;
				}
				else {
					quadNode->triangulationMode = TM_SW_NE;
				}
			}
		}
	}

	auto QuadCollapseMesh::getVertNode(uint32_t level, int32_t x, int32_t y, bool init_mode) -> VertNode*
	{
		auto vertNode = vertNodePool[vertNodesLevelOffset[level]];

		uint32_t shift = maxLevel - level;
		uint32_t rowStep = (1 << level) + 1;

		x >>= shift;
		y >>= shift;

		//auto result = vertNode + y * rowStep + x;
		auto result = &vertNodePool[vertNodesLevelOffset[level] + y * rowStep + x];

		if (!init_mode) { // do check
			if (result->level != level) {
				LOGE("get vert node error");
			}
		}

		return result;
	}

	auto QuadCollapseMesh::allocQuadNode() -> QuadNode*
	{
		if (quadNodePoolAllocated >= quadNodePool.size()) {
			LOGE("quad node pool overflow\n");
			return nullptr;
		}
		else {
			return &quadNodePool[quadNodePoolAllocated++];
		}
	}

	auto QuadCollapseMesh::allocQuadLeaf() -> QuadLeaf*
	{
		if (quadLeafPoolAllocated >= quadLeafPool.size()) {
			LOGE("quad leaf pool overflow\n");
			return nullptr;
		}
		else {
			return &quadLeafPool[quadLeafPoolAllocated++];
		}
	}

	auto QuadCollapseMesh::recursiveUpdateVertNode(Camera* camera, VertNode* vertNode) -> void
	{
		if (vertNode->activeFrame == updateFrame) {
			return;
		}

		vertNode->activeFrame = updateFrame;
		vertNode->state = NS_BOUNDARY;

		auto delta = glm::vec3(1) - vertNode->originalVertex.pos;
		float dist = glm::length(delta);

		if (dist < vertNodesActiveDistance[vertNode->level]) {
			if (vertNode->firstChild) {
				vertNode->state = NS_ACTIVE;

				auto child = vertNode->firstChild;
				while (child) {
					if (child->parent != vertNode) {
						LOGE("child->parent != vert_node\n");
					}
					recursiveUpdateVertNode(camera, child);
					child = child->nextSibling;
				}
			}
			else {
				vertNode->interpolatedVertex= vertNode->originalVertex;
				for (int32_t i = 0; i < (int32_t)vertNode->adjcentQuadsCount; ++i) {
					quadNodeSetBoundary(camera, vertNode->adjcentQuads[i]);
				}
			}
		}
		else {
			if (vertNode->parent) {
				// interpolate
	
				auto p = vertNode->parent;

				auto o_minus_c = vertNode->originalVertex - p->originalVertex;

				auto l = glm::vec3(1.f) - vertNode->originalVertex.pos;
				l = glm::normalize(l);

				float l_dot_o_minus_c = glm::dot(l, o_minus_c.pos);

				float r = vertNodesActiveDistance[vertNode->level - 1];

				float sqrLength = glm::dot(o_minus_c.pos, o_minus_c.pos);
				float temp = (l_dot_o_minus_c * l_dot_o_minus_c) - sqrLength + r * r;

				float d = -l_dot_o_minus_c + std::sqrtf(temp);
				float t = (d - dist) / (d - vertNodesActiveDistance[vertNode->level]);

				auto pOrigin = p->originalVertex;
				auto cOrigin = vertNode->originalVertex;

				vertNode->interpolatedVertex = pOrigin +  (cOrigin - pOrigin) * t;


				for (uint32_t i = 0; i < vertNode->adjcentQuadsCount; ++i) {
					quadNodeSetBoundary(camera, vertNode->adjcentQuads[i]);
				}
			}
			else {
				vertNode->interpolatedVertex = vertNode->originalVertex;
				for (uint32_t i = 0; i < vertNode->adjcentQuadsCount; ++i) {
					quadNodeSetBoundary(camera, vertNode->adjcentQuads[i]);
				}
			}
		}
	}

	auto QuadCollapseMesh::quadNodeSetBoundary(Camera* camera, QuadNode* quadNode) -> void
	{
		if (quadNode->activeFrame == updateFrame && quadNode->state == NS_ACTIVE) {
			return; // already set
		}

		

		quadNode->activeFrame = updateFrame;
		quadNode->state = NS_BOUNDARY;

		auto p = quadNode->parent;
		while (p) {
			if (p->activeFrame == updateFrame && p->state == NS_ACTIVE) {
				break;
			}

			p->activeFrame = updateFrame;
			p->state = NS_ACTIVE;
			p = p->parent;
		}
	}

	auto QuadCollapseMesh::recursiveSetActiveMesh(QuadNode* quadNode) -> void
	{
		if (!quadNode) {
			return;
		}

		if (quadNode->activeFrame == updateFrame) {
			if (quadNode->state == NS_BOUNDARY) {

				if (quadNode->triangulationMode == TM_SW_NE) {
					addActiveVertNode(quadNode->cornerVertNodes[0]);
					addActiveVertNode(quadNode->cornerVertNodes[1]);
					addActiveVertNode(quadNode->cornerVertNodes[2]);
					addActiveVertNode(quadNode->cornerVertNodes[0]);
					addActiveVertNode(quadNode->cornerVertNodes[2]);
					addActiveVertNode(quadNode->cornerVertNodes[3]);
				}
				else {
					addActiveVertNode(quadNode->cornerVertNodes[0]);
					addActiveVertNode(quadNode->cornerVertNodes[1]);
					addActiveVertNode(quadNode->cornerVertNodes[3]);
					addActiveVertNode(quadNode->cornerVertNodes[1]);
					addActiveVertNode(quadNode->cornerVertNodes[2]);
					addActiveVertNode(quadNode->cornerVertNodes[3]);
				}
			}
			else { // NS_ACTIVE
				for (int32_t i = 0; i < 4; ++i) {
					recursiveSetActiveMesh(quadNode->children[i]);
				}
			}
		}
		else { // not active, but a child of active quad
			if (quadNode->triangulationMode == TM_SW_NE) {
				addActiveVertNode(quadNode->cornerVertNodes[0]);
				addActiveVertNode(quadNode->cornerVertNodes[1]);
				addActiveVertNode(quadNode->cornerVertNodes[2]);
				addActiveVertNode(quadNode->cornerVertNodes[0]);
				addActiveVertNode(quadNode->cornerVertNodes[2]);
				addActiveVertNode(quadNode->cornerVertNodes[3]);
			}
			else {
				addActiveVertNode(quadNode->cornerVertNodes[0]);
				addActiveVertNode(quadNode->cornerVertNodes[1]);
				addActiveVertNode(quadNode->cornerVertNodes[3]);
				addActiveVertNode(quadNode->cornerVertNodes[1]);
				addActiveVertNode(quadNode->cornerVertNodes[2]);
				addActiveVertNode(quadNode->cornerVertNodes[3]);
			}
		}
	}

	auto QuadCollapseMesh::addActiveVertNode(VertNode* vertNode) -> void
	{
		if (vertNode->activeFrame == updateFrame) {
			activeVertices.emplace_back(vertNode->interpolatedVertex);
		}
		else {
			Vertex * pos = nullptr;
			auto p = vertNode->parent;

			while (p) {
				if (p->activeFrame == updateFrame) {
					pos = &p->interpolatedVertex;
					break;
				}
				p = p->parent;
			}

			if (pos) {
				activeVertices.emplace_back(*pos);
			}
			else {
				return;
			}
		}
	}

};

