//////////////////////////////////////////////////////////////////////////////
// This file is part of the Raven Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <imgui.h>
#include <string>
#include <glm/glm.hpp>
#include "Engine/Core.h"
namespace Maple 
{

	class Texture;

	namespace ImGuiHelper
	{
		enum class PropertyFlag
		{
			None = 0,
			ColorProperty = 1
		};

		MAPLE_EXPORT auto tooltip(const char* str) -> void;
		MAPLE_EXPORT auto property(const std::string& name, bool& value) -> bool;
		MAPLE_EXPORT auto property(const std::string& name, std::string& value, bool disable = false)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, float& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, int32_t& value, int32_t min = -1, int32_t max = 1, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, uint32_t& value, uint32_t min = 0, uint32_t max = 1, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec2& value, PropertyFlag flags)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec2& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec3& value, PropertyFlag flags)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec3& value, float min = -1.0f, float max = 1.0f, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec4& value, bool exposeW, PropertyFlag flags)-> bool;
		MAPLE_EXPORT auto property(const std::string& name, glm::vec4& value, float min = -1.0f, float max = 1.0f, bool exposeW = false, PropertyFlag flags = PropertyFlag::None)-> bool;
		MAPLE_EXPORT auto inputFloat(const std::string& name, float& value, float min = -1.0f, float max = 1.0f)-> bool;
		MAPLE_EXPORT auto image(const Texture* texture, const glm::vec2& size) -> void;
	};

};



static inline ImVec2 operator*(const ImVec2& lhs, const float rhs)   { return ImVec2(lhs.x*rhs, lhs.y*rhs); }
static inline ImVec2 operator/(const ImVec2& lhs, const float rhs)   { return ImVec2(lhs.x/rhs, lhs.y/rhs); }
static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline ImVec2 operator*(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline ImVec2 operator/(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x/rhs.x, lhs.y/rhs.y); }
static inline ImVec2& operator+=(ImVec2& lhs, const ImVec2& rhs)     { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline ImVec2& operator-=(ImVec2& lhs, const ImVec2& rhs)     { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline ImVec2& operator*=(ImVec2& lhs, const float rhs)       { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec2& operator/=(ImVec2& lhs, const float rhs)       { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
static inline ImVec4 operator-(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x-rhs.x, lhs.y-rhs.y, lhs.z-rhs.z, lhs.w-rhs.w); }
static inline ImVec4 operator+(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z, lhs.w+rhs.w); }
static inline ImVec4 operator*(const ImVec4& lhs, const float rhs)   { return ImVec4(lhs.x*rhs, lhs.y*rhs,lhs.z*rhs,lhs.w*rhs); }
static inline ImVec4 operator/(const ImVec4& lhs, const float rhs)   { return ImVec4(lhs.x/rhs, lhs.y/rhs,lhs.z/rhs,lhs.w/rhs); }
static inline ImVec4 operator*(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x*rhs.x, lhs.y*rhs.y,lhs.z*rhs.z,lhs.w*rhs.w); }
static inline ImVec4 operator/(const ImVec4& lhs, const ImVec4& rhs) { return ImVec4(lhs.x/rhs.x,lhs.y/rhs.y,lhs.z/rhs.z,lhs.w/rhs.w); }
static inline ImVec4& operator+=(ImVec4& lhs, const ImVec4& rhs)     { lhs.x += rhs.x; lhs.y += rhs.y;lhs.z += rhs.z;lhs.w += rhs.w;return lhs; }
static inline ImVec4& operator-=(ImVec4& lhs, const ImVec4& rhs)     { lhs.x -= rhs.x; lhs.y -= rhs.y;lhs.z -= rhs.z;lhs.w -= rhs.w; return lhs; }
static inline ImVec4& operator*=(ImVec4& lhs, const float rhs)       { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline ImVec4& operator/=(ImVec4& lhs, const float rhs)       { lhs.x /= rhs; lhs.y /= rhs; return lhs; }
