//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef BIT
#define BIT(x) (1 << x)
#endif
#include <memory>
#include <glm/glm.hpp>

namespace Maple
{
	class Mesh;
	class Material;

	enum RendererBufferType
	{
		RENDERER_BUFFER_NONE = 0,
		RENDERER_BUFFER_COLOR = BIT(0),
		RENDERER_BUFFER_DEPTH = BIT(1),
		RENDERER_BUFFER_STENCIL = BIT(2)
	};

	enum class DrawType
	{
		POINT,
		TRIANGLE,
		TRIANGLE_STRIP,
		LINES
	};

	enum class StencilType
	{
		EQUAL,
		NOTEQUAL,
		KEEP,
		REPLACE,
		ZERO,
		ALWAYS
	};

	enum class PixelPackType
	{
		PACK,
		UNPACK
	};

	enum class RendererBlendFunction
	{
		NONE,
		ZERO,
		ONE,
		SOURCE_ALPHA,
		DESTINATION_ALPHA,
		ONE_MINUS_SOURCE_ALPHA
	};

	enum class RendererBlendEquation
	{
		NONE,
		ADD,
		SUBTRACT
	};

	enum class RenderMode
	{
		FILL,
		WIREFRAME
	};

	enum class DataType
	{
		FLOAT,
		UNSIGNED_INT,
		UNSIGNED_BYTE
	};

	enum class CullMode
	{
		FRONT,
		BACK,
		FRONTANDBACK,
		NONE
	};

	enum class PolygonMode
	{
		FILL,
		LINE,
		POINT
	};


	struct  RenderCommand
	{
		Mesh* mesh = nullptr;
		std::shared_ptr<Material> material;
		glm::mat4 transform;
	};
};