//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "File.h"
#include <sys/stat.h>
#include <exception>
#include <cstdio>
#include <cstring>
#include <memory>
#include <filesystem>
#ifdef __ANDROID__
#include <dirent.h>
#endif // __ANDROID__
#include "Others/Console.h"

#include "Others/StringUtils.h"

namespace Maple
{
	// 128 MB.
	const size_t CHUNK_SIZE = 134217728;

	File::~File()
	{
		if (filePtr != nullptr) {
			fclose(filePtr);
		}
		//stream.close();
	}

	File::File() : fileSize(0)
	{
	}

	File::File(const std::string& file,bool write) : file(file)
	{
		
		if(!write)
		{
			filePtr = fopen(file.c_str(), "rb");
			if(filePtr!=nullptr)
			{
				fseek(filePtr, 0, SEEK_END);
				fileSize = ftell(filePtr);
				fseek(filePtr, 0, SEEK_SET);
			}
		}
		else
		{
			filePtr = fopen(file.c_str(), "wb+");
		}
	
	}
    
    
    auto File::getBuffer() -> std::unique_ptr<int8_t[]>
    {
        return readBytes(fileSize);
    }

	auto File::write(const std::string& file) -> void
	{
		fwrite(file.c_str(), 1, file.length() + 1, filePtr);
	}


	auto File::getMd5() -> std::string
	{
		return "";
	}

	auto File::exists() -> bool
	{
		return fileExists(file);
	}

	auto File::isDirectory() -> bool
	{
		struct stat fileInfo;
		auto result = stat(file.c_str(), &fileInfo);
		if (result != 0)
		{
			throw std::logic_error("file not exits");
		}
		return (fileInfo.st_mode & S_IFDIR) == S_IFDIR;
	}


	auto File::readBytes(int32_t size) -> std::unique_ptr<int8_t[]>
	{
		std::unique_ptr<int8_t[]> array(new int8_t[size]);
		memset(array.get(), 0, size);
		fread(array.get(), sizeof(int8_t) * size, 1, filePtr);
		pos += sizeof(int8_t) * size;
		return array;
	}

	auto File::removeExtension(const std::string& file) -> std::string
	{
        if(file == ""){
            return "";
        }
		std::string ret = file;
		ret = ret.erase(ret.find_last_of('.'));
		return ret;
	}

	auto File::read(const std::string& name) ->std::vector<uint8_t>
	{
		std::ifstream file(name, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

 		size_t fileSize = (size_t)file.tellg();
		std::vector<uint8_t> buffer(fileSize);

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

		file.close();

		return buffer;
	}

	auto File::getFileType(const std::string& path) ->FileType
	{
		static std::unordered_map<std::string, FileType> types = {
			{"mp3",FileType::MP3},
			{"ogg",FileType::OGG},
			{"wav",FileType::WAV},
			{"aac",FileType::AAC},
			{"jpg",FileType::Texture},
			{"png",FileType::Texture},
			{"tga",FileType::Texture},
			{"lua",FileType::Script},
			{"cs",FileType::C_SHARP},
			{"glsl",FileType::Text},
			{"shader",FileType::Text},
			{"vert",FileType::Text},
			{"frag",FileType::Text},
			{"text",FileType::Text},
			{"scene",FileType::Scene},
			{"obj",FileType::Model},
			{"fbx",FileType::Model},
			{"glb",FileType::Model},
			{"gltf",FileType::Model},
			{"dll",FileType::Dll},
			{"so",FileType::Dll}
		};

		auto extension = StringUtils::getExtension(path);
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) -> unsigned char { return std::tolower(c); });
		if (auto iter = types.find(extension); iter != types.end()) {
			return iter->second;
		}
		return FileType::Normal;
	}

	auto File::cache(const std::vector<uint8_t>& buffer) -> void
	{
		if(fileSize != buffer.size())
		{
			fwrite(buffer.data(),sizeof(uint8_t) * buffer.size(),1,filePtr);
		}
	}

	auto File::fileExists(const std::string& file) -> bool
	{
		struct stat fileInfo;
		return (!stat(file.c_str(), &fileInfo)) != 0;
	}

	auto File::list() -> const std::vector<std::string> {
		std::vector<std::string> files;
		list(files);
		return files;
	}

	auto File::list(std::vector<std::string> &out) -> void {
		if (file.empty()) {
			return ;
		}
	}

    auto File::getFileName(const std::string& file) -> const std::string {
		auto pos = file.find_last_of('/');
		return file.substr(pos+1);
	}

}; // namespace FileSystem
