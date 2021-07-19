//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////


#pragma once
#include <string>
#include <vector>
#include <stdarg.h>
#include <functional>
#include <memory>

namespace Maple 
{

	namespace StringUtils
	{

		template <typename... Args>
		auto format(const std::string& format, Args... args) -> std::string
		{
			size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
			std::unique_ptr<char[]> buf(new char[size]);
			snprintf(buf.get(), size, format.c_str(), args...);
			return std::string(buf.get(), buf.get() + size - 1);
		}

		auto replace(std::u16string& src, const std::u16string& origin, const std::u16string& des) -> void;
		auto split(std::string input, const std::string& delimiter)->std::vector<std::string>;
		auto split(std::u16string input, const std::u16string& delimiter, std::vector<std::u16string>& outs) -> void;
		auto split(std::string input, const std::string& delimiter, std::vector<std::string>& outs) -> void;
		auto startWith(const std::string& str, const std::string& start) -> bool;
		auto contains(const std::string& str, const std::string& start) -> bool;
		auto endWith(const std::string& str, const std::string& start) -> bool;
		auto trim(std::string& str,const std::string & trimStr = " ") -> void;
		auto trim(std::u16string& str) -> void;
		auto replace(std::string& str, const std::string& old, const std::string& newStr) -> void;

		auto toLower(std::string& data) -> void;


		auto getExtension(const std::string& fileName) -> std::string;
		auto removeExtension(const std::string& fileName)->std::string;
		auto getFileName(const std::string& filePath)->std::string;
		auto getFileNameWithoutExtension(const std::string& filePath)->std::string;
		auto getCurrentWorkingDirectory()->std::string;

		auto isHiddenFile(const std::string& filePath)->bool;
		auto isTextFile(const std::string& filePath) -> bool;
		auto isLuaFile(const std::string& filePath) -> bool;
		auto isAudioFile(const std::string& filePath) -> bool;
		auto isSceneFile(const std::string& filePath) -> bool;
		auto isControllerFile(const std::string& filePath)  -> bool;
		auto isModelFile(const std::string& filePath) -> bool;
		auto isTextureFile(const std::string& filePath) -> bool;

#ifdef _WIN32
		static const std::string delimiter = "\\";
#else
		static const std::string delimiter = "/";
#endif
	}; // namespace StringUtils
};

