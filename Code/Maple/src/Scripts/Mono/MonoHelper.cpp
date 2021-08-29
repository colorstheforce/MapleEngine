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

	auto compileScript(const std::vector<std::string>& files, const std::string& dllReference /*= ""*/) -> bool
	{
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
		}
		return compilationResult;
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

	auto getClassName(MonoObject* obj, std::string& ns, std::string& typeName) -> void
	{
		if (obj == nullptr)
			return;
		getClassName(mono_object_get_class(obj), ns, typeName);
	}

	auto getClassName(MonoClass* monoClass, std::string& ns, std::string& typeName) -> void
	{
		auto nestingClass = mono_class_get_nesting_type(monoClass);

		if (nestingClass == nullptr)
		{
			ns = mono_class_get_namespace(monoClass);
			typeName = mono_class_get_name(monoClass);

			return;
		}
		else
		{
			const char* className = mono_class_get_name(monoClass);

			if (className)
				typeName = std::string("+") + className;

			do
			{
				auto nextNestingClass = mono_class_get_nesting_type(nestingClass);
				if (nextNestingClass != nullptr)
				{
					typeName = std::string("+") + mono_class_get_name(nestingClass) + typeName;
					nestingClass = nextNestingClass;
				}
				else
				{
					ns = mono_class_get_namespace(nestingClass);
					typeName = mono_class_get_name(nestingClass) + typeName;
					break;
				}
			} while (true);
		}
	}

	auto getClassName(MonoReflectionType* monoReflType, std::string& ns, std::string& typeName) -> void
	{
		getClassName(mono_type_get_class(mono_reflection_type_get_type(monoReflType)), ns, typeName);
	}

	auto monoToString(MonoString* str) -> std::string
	{
		if (str == nullptr)
			return "";
		auto chars = mono_string_to_utf8(str);
		std::string ret = chars;
		mono_free(chars);
		return ret;
	}

	auto stringToMono(const std::string& str) -> MonoString*
	{
		auto utf16 = StringUtils::UTF8ToUTF16(str);
		return mono_string_from_utf16((mono_unichar2*)str.c_str());
	}

	auto throwIfException(MonoObject* exception) -> void
	{
		if (exception != nullptr)
		{
			MonoClass* exceptionClass = mono_object_get_class(exception);
			MonoProperty* exceptionMsgProp = mono_class_get_property_from_name(exceptionClass, "Message");
			MonoMethod* exceptionMsgGetter = mono_property_get_get_method(exceptionMsgProp);
			MonoString* exceptionMsg = (MonoString*)mono_runtime_invoke(exceptionMsgGetter, exception, nullptr, nullptr);

			MonoProperty* exceptionStackProp = mono_class_get_property_from_name(exceptionClass, "StackTrace");
			MonoMethod* exceptionStackGetter = mono_property_get_get_method(exceptionStackProp);
			MonoString* exceptionStackTrace = (MonoString*)mono_runtime_invoke(exceptionStackGetter, exception, nullptr, nullptr);
			// Note: If you modify this format make sure to also modify Debug.ParseExceptionMessage in managed code.
			auto msg = "Managed exception: " + monoToString(exceptionMsg) + "\n" + monoToString(exceptionStackTrace);
			LOGE(msg);
		}
	}

	auto getUINT16Class() -> MonoClass*
	{
		return mono_get_uint16_class();
	}

	auto getINT16Class() -> MonoClass*
	{
		return mono_get_int16_class();
	}

	auto getUINT32Class() -> MonoClass*
	{
		return mono_get_uint32_class();
	}

	auto getINT32Class() -> MonoClass*
	{
		return mono_get_int32_class();
	}

	auto getUINT64Class() -> MonoClass*
	{
		return mono_get_uint64_class(); 
	}

	auto getINT64Class() -> MonoClass*
	{
		return mono_get_int64_class();
	}

	auto getStringClass() -> MonoClass*
	{
		return mono_get_string_class();
	}

	auto getFloatClass() -> MonoClass*
	{
		return mono_get_single_class();
	}

	auto getDoubleClass() -> MonoClass*
	{
		return mono_get_double_class();
	}

	auto getBoolClass() -> MonoClass*
	{
		return mono_get_boolean_class();
	}

	auto getByteClass() -> MonoClass*
	{
		return mono_get_byte_class();
	}

	auto getSByteClass() -> MonoClass*
	{
		return mono_get_sbyte_class();
	}

	auto getCharClass() -> MonoClass*
	{
		return mono_get_char_class();
	}

	auto getObjectClass() -> MonoClass*
	{
		return mono_get_object_class();
	}

};