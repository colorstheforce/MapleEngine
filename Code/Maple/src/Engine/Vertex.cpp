
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "Vertex.h"
namespace Maple
{
	
	auto Vertex::operator-(const Vertex& right) ->Vertex
	{
		Vertex ret;
		ret.pos = pos - right.pos;
		ret.color = color - right.color;
		ret.texCoord = texCoord - right.texCoord;
		ret.normal = normal - right.normal;
		return ret;
	}

	auto Vertex::operator+(const Vertex& right) ->Vertex
	{
		Vertex ret;
		ret.pos = pos + right.pos;
		ret.color = color + right.color;
		ret.texCoord = texCoord + right.texCoord;
		ret.normal = normal + right.normal;
		return ret;
	}

	auto Vertex::operator*(float factor) ->Vertex
	{
		Vertex ret;
		ret.pos = pos * factor;
		ret.color = color * factor;
		ret.texCoord = texCoord * factor;
		ret.normal = normal * factor;
		return ret;
	}

	auto Vertex::operator==(const Vertex& other) const -> bool {
		return pos == other.pos && color == other.color && texCoord == other.texCoord
			&& normal == other.normal && tangent == other.tangent;
	}

	auto Vertex2D::operator==(const Vertex2D& other) const-> bool
	{
		return vertex == other.vertex && uv == other.uv && color == other.color;
	}

};