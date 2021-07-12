

#include <ctime>
#include <stdio.h>
#include <string>
#include <sstream>
#include <memory.h>
#include <algorithm>
#include "StringUtils.h"
#include "Console.h"


#ifdef _WIN32
#include <direct.h>
#define getCwd _getcwd
#else
#include <unistd.h>
#define getCwd getcwd
#endif



namespace Maple 
{
	auto StringUtils::getExtension(const std::string& fileName) -> std::string
	{
		auto pos = fileName.find_last_of('.');
		if (pos != std::string::npos)
			return fileName.substr(pos + 1);
		return "";
	}

	auto StringUtils::removeExtension(const std::string& fileName) -> std::string
	{
		auto pos = fileName.find_last_of('.');
		if (pos != std::string::npos)
			return fileName.substr(0, pos);
		return fileName;
	}

	auto StringUtils::getFileName(const std::string& filePath) -> std::string
	{
		auto pos = filePath.find_last_of(delimiter);
		if (pos != std::string::npos)
			return filePath.substr(pos + 1);

		return filePath;
	}

	auto StringUtils::getFileNameWithoutExtension(const std::string& filePath) -> std::string
	{
		return removeExtension(getFileName(filePath));
	}

	auto StringUtils::getCurrentWorkingDirectory() ->std::string
	{
		char currentPath[FILENAME_MAX];
		if (!getCwd(currentPath, sizeof(currentPath)))
		{
			return std::string(); // empty string
		}
		currentPath[sizeof(currentPath) - 1] = '\0'; // terminate the string
		return std::string(currentPath);
	}

	auto StringUtils::isHiddenFile(const std::string& filePath) ->bool
	{
		if (filePath != ".." &&
			filePath != "." &&
			filePath[0] == '.')
		{
			return true;
		}
		return false;
	}

	auto StringUtils::replace(std::u16string& src, const std::u16string& origin, const std::u16string& des) -> void
	{
		std::u16string::size_type pos = 0;
		std::u16string::size_type srcLen = origin.size();
		std::u16string::size_type desLen = des.size();
		pos = src.find(origin, pos);
		while ((pos != std::u16string::npos))
		{
			src.replace(pos, srcLen, des);
			pos = src.find(origin, (pos + desLen));
		}
	}

	auto StringUtils::split(std::string input, const std::string& delimiter) -> std::vector<std::string>
	{
		std::vector<std::string> ret;
		size_t pos = 0;
		std::string token;
		while ((pos = input.find(delimiter)) != std::string::npos)
		{
			token = input.substr(0, pos);
			ret.push_back(token);
			input.erase(0, pos + delimiter.length());
		}
		ret.push_back(input);
		return ret;
	}

	auto StringUtils::startWith(const std::string& str, const std::string& start) -> bool
	{
		return str.compare(0, start.size(), start) == 0;
	}

	auto StringUtils::contains(const std::string& str, const std::string& start) -> bool
	{
		return str.find(start) != std::string::npos;
	}

	auto StringUtils::endWith(const std::string& str, const std::string& start) -> bool
	{
		return str.compare(str.length() - start.length(), start.size(), start) == 0;
	}

	auto StringUtils::split(std::string input, const std::string& delimiter, std::vector<std::string>& outs) -> void
	{
		size_t pos = 0;
		std::string token;
		while ((pos = input.find(delimiter)) != std::string::npos)
		{
			token = input.substr(0, pos);
			outs.push_back(token);
			input.erase(0, pos + delimiter.length());
		}
		outs.push_back(input);
	}

	auto StringUtils::split(std::u16string input, const std::u16string& delimiter,
		std::vector<std::u16string>& outs) -> void
	{
		size_t pos = 0;
		std::u16string token;
		while ((pos = input.find(delimiter)) != std::u16string::npos)
		{
			token = input.substr(0, pos);
			outs.push_back(token);
			input.erase(0, pos + delimiter.length());
		}
		outs.push_back(input);
	}


	auto StringUtils::trim(std::string& str, const std::string& trimStr) -> void
	{
		if (!str.empty())
		{
			str.erase(0, str.find_first_not_of(trimStr));
			str.erase(str.find_last_not_of(trimStr) + 1);
		}
	}

	auto StringUtils::replace(std::string& str, const std::string& old, const std::string& newStr) -> void
	{
		std::string::size_type pos = 0;
		std::string::size_type srclen = old.size();
		std::string::size_type dstlen = newStr.size();

		while ((pos = str.find(old, pos)) != std::string::npos)
		{
			str.replace(pos, srclen, newStr);
			pos += dstlen;
		}
	}

	auto StringUtils::trim(std::u16string& str) -> void
	{
		if (!str.empty())
		{
			str.erase(0, str.find_first_not_of(u" "));
			str.erase(str.find_last_not_of(u" ") + 1);
			str.erase(0, str.find_first_not_of(u"\u3000"));
			str.erase(str.find_last_not_of(u"\u3000") + 1);
		}
	}

	auto StringUtils::isTextFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		if (extension == "txt" || extension == "glsl" || extension == "shader" || extension == "vert"
			|| extension == "frag" || extension == "lua")
			return true;

		return false;
	}

	auto StringUtils::isLuaFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "lua";
	}

	auto StringUtils::isAudioFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "ogg" || extension == "mp3" || extension == "wav" || extension == "aac";
	}

	auto StringUtils::isSceneFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "scene";

	}

	auto StringUtils::isControllerFile(const std::string& filePath)  -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "controller";
	}

	auto StringUtils::isModelFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "obj" || extension == "gltf" || extension == "glb" || extension == "fbx" || extension == "FBX";
	}

	auto StringUtils::isTextureFile(const std::string& filePath) -> bool
	{
		std::string extension = getExtension(filePath);
		trim(extension);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		return extension == "png" || extension == "tga" || extension == "jpg" || extension == "hdr";
	}


};