//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine									//
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#include "MonoHelper.h"
#include "Others/Console.h"
#include "Others/StringUtils.h"
#include <array>
#include <memory>
#include <iostream>
#include <sstream>

namespace Maple::MonoHelper 
{
	auto executeCommand(const std::string & cmd)->std::string
	{
		std::array<char, 1024> buffer;
		std::string result;
		const std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
		if (!pipe)
		{
			LOGE("open file fail {0}",cmd);
			return result;
		}

		while (fgets(buffer.data(), static_cast<int32_t>(buffer.size()), pipe.get()) != nullptr)
		{
			result += buffer.data();
		}
		return result;
	}

	auto compileScript(const std::string& script, const std::string& dllReference /*= ""*/) -> bool
	{
		const std::string dirScripts = "scripts/";
#ifdef _WIN32
		const std::string dirCompiler = "..\\Toolchains\\mono\\roslyn\\csc.exe";
#else

#endif // _WIN32

		std::string command = dirCompiler + " -target:library -nologo";
		if (!dllReference.empty())
		{
			command += " -reference:" + dllReference;
		}
		command += " -out:" + StringUtils::replaceExtension(script, ".dll") + " " + std::string(script);

		std::istringstream f(executeCommand(command.c_str()));
		std::string line;
		bool compilationResult = true;
		while (std::getline(f, line))
		{
			if (StringUtils::isEmptyOrWhitespace(line))
				continue;

			const auto is_error = line.find("error") != std::string::npos;
			if (is_error)
			{
				LOGE(line);
				compilationResult = false;
			}
			else
			{
				LOGI(line);
			}
		}

		if (compilationResult)
		{
			LOGI("Successfully compiled C# Script \"{0}\"", script);
			return true;
		}
		return false;
	}

	auto compileScript(MonoDomain* domain, const std::vector<std::string>& files, const std::string& dllReference /*= ""*/) -> MonoAssembly *
	{
		const std::string dirScripts = "scripts/";
#ifdef _WIN32
		const std::string dirCompiler = "..\\Toolchains\\mono\\roslyn\\csc.exe";
#else

#endif // _WIN32
		std::string script = "";

		for (auto & f : files)
		{
			script.append(f);
			script.append(" ");
		}

		std::string command = dirCompiler + " -target:library -nologo";
		if (!dllReference.empty())
		{
			command += " -reference:" + dllReference;
		}
		command += " -out:MapleAssembly.dll " + script;

		std::istringstream f(executeCommand(command.c_str()));
		std::string line;
		bool compilationResult = true;
		while (std::getline(f, line))
		{
			if (StringUtils::isEmptyOrWhitespace(line))
				continue;

			const auto is_error = line.find("error") != std::string::npos;
			if (is_error)
			{
				LOGE(line);
				compilationResult = false;
			}
			else
			{
				LOGI(line);
			}
		}

		if (compilationResult)
		{
			LOGI("Successfully compiled C# Script \"{0}\"", script);
			return mono_domain_assembly_open(domain, "MapleAssembly.dll");
		}
		return nullptr;
	}


	auto getMethod(MonoImage* image, const std::string& method) -> MonoMethod*
	{
		MonoMethodDesc* methodDesc = mono_method_desc_new(method.c_str(), NULL);
		if (!methodDesc)
		{
			LOGE("Failed to get method description {0}", method);
			return nullptr;
		}

		MonoMethod* monoMethod = mono_method_desc_search_in_image(methodDesc, image);
		if (!monoMethod)
		{
			LOGE("Failed to get method {0}", method);
			return nullptr;
		}
		return monoMethod;
	}

};