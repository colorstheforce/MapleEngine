//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>

namespace Maple
{

	enum class FileType : int32_t
	{
		Normal,
		Folder,
		Texture,
		Model,
		FBX,
		OBJ,
		Text,
		Script,
		Dll,
		Scene,
		MP3,
		OGG,
		AAC,
		WAV,
		TTF,
		C_SHARP,
		Length
	};



	class File
	{
	public:
		File();
		explicit File(const std::string& file,bool write = false);
		~File();
		auto exists() -> bool;
		auto getMd5() -> std::string;
		auto isDirectory() -> bool;
		auto getFileSize() { return fileSize; }
		auto getOffset() { return pos; }
		auto readBytes(int32_t size)->std::unique_ptr<int8_t[]>;
		auto list() -> const std::vector<std::string>;
		auto list(std::vector<std::string> & out) -> void;
		auto cache(const std::vector<uint8_t> & buffer) -> void;
        auto getBuffer() -> std::unique_ptr<int8_t[]>;
		auto write(const std::string& file) -> void;
		static auto getFileName(const std::string& file) -> const std::string;
		static auto fileExists(const std::string& file) -> bool;
		static auto removeExtension(const std::string& file) -> std::string;
		static auto read(const std::string& name)->std::vector<uint8_t>;

		static auto getFileType(const std::string& path)->FileType;

	private:
		std::string file;
		//std::ifstream stream;
		FILE* filePtr = nullptr;
		int64_t pos = 0;
		size_t fileSize;
	public:
	};
}; // namespace FileSystem
