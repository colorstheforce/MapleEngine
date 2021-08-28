
#include "MapleMonoAssembly.h"
#include "MapleMonoClass.h"
#include "MonoHelper.h"
#include "MonoVirtualMachine.h"

#include "Others/HashCode.h"
#include "Others/Console.h"
#include "FileSystem/File.h"
#include "Others/StringUtils.h"
#include <stack>

namespace Maple 
{

	MapleMonoAssembly::~MapleMonoAssembly()
	{
		unload();
	}

	MapleMonoAssembly::MapleMonoAssembly(const std::string& path, const std::string& name)
		: name(name), path(path)
	{

	}

	auto MapleMonoAssembly::getClass(const std::string& namespaceName, const std::string& name) const ->std::shared_ptr<MapleMonoClass>
	{
		if (!loaded) {
			LOGE("Trying to use an unloaded assembly.");
		}

		ClassId classId(namespaceName, name);
		auto iterFind = classes.find(classId);
		if (iterFind != classes.end())
			return iterFind->second;

		auto monoClass = mono_class_from_name(monoImage, namespaceName.c_str(), name.c_str());
		if (monoClass == nullptr)
			return nullptr;

		auto newClass = std::make_shared<MapleMonoClass>(namespaceName, name, monoClass, this);
		classes[classId] = newClass;
		classesByRaw[monoClass] = newClass;
		return newClass;
	}
	auto MapleMonoAssembly::getClass(MonoClass* rawMonoClass) const ->std::shared_ptr<MapleMonoClass>
	{
		if (!loaded) {
			LOGE("Trying to use an unloaded assembly.");
		}

		if (rawMonoClass == nullptr)
			return nullptr;

		auto iterFind = classesByRaw.find(rawMonoClass);

		if (iterFind != classesByRaw.end())
			return iterFind->second;

		std::string ns;
		std::string typeName;
		MonoHelper::getClassName(rawMonoClass, ns, typeName);
		MonoImage* classImage = mono_class_get_image(rawMonoClass);
		if (classImage != monoImage)
			return nullptr;

		auto newClass = std::make_shared<MapleMonoClass>(ns, typeName, rawMonoClass, this);
		classesByRaw[rawMonoClass] = newClass;
		ClassId classId(ns, typeName);
		classes[classId] = newClass;

		return newClass;
	}

	auto MapleMonoAssembly::getClass(const std::string& namespaceName, const std::string& name, MonoClass* rawMonoClass) const -> std::shared_ptr<MapleMonoClass>
	{
		if (!loaded) {
			LOGE("Trying to use an unloaded assembly.");
		}

		if (rawMonoClass == nullptr)
			return nullptr;

		auto iterFind = classesByRaw.find(rawMonoClass);

		if (iterFind != classesByRaw.end())
			return iterFind->second;


		auto newClass = std::make_shared<MapleMonoClass>(namespaceName, name, rawMonoClass, this);

		classesByRaw[rawMonoClass] = newClass;

		if (!isGenericClass(name)) 
		{
			ClassId classId(namespaceName, name);
			classes[classId] = newClass;
		}

		return newClass;
	}

	auto MapleMonoAssembly::getAllClasses() const -> const std::vector<std::shared_ptr<MapleMonoClass>>&
	{
		if (haveCachedClassList)
			return cachedClassList;

		cachedClassList.clear();
		std::stack<std::shared_ptr<MapleMonoClass>> todo;

		auto corlib = MonoVirtualMachine::get()->getAssembly("corlib");
		auto compilerGeneratedAttrib = corlib->getClass(
			"System.Runtime.CompilerServices",
			"CompilerGeneratedAttribute"
		);

		auto numRows = mono_image_get_table_rows(monoImage, MONO_TABLE_TYPEDEF);

		for (int32_t i = 1; i < numRows; i++) // Skip Module
		{
			auto monoClass = mono_class_get(monoImage, (i + 1) | MONO_TOKEN_TYPE_DEF);

			std::string ns;
			std::string type;
			MonoHelper::getClassName(monoClass, ns, type);

			auto curClass = getClass(ns, type);
			if (curClass != nullptr)
			{
				// Skip compiler generates classes
				if (curClass->hasAttribute(compilerGeneratedAttrib))
					continue;

				// Get nested types if it has any
				todo.push(curClass);
				while (!todo.empty())
				{
					auto curNestedClass = todo.top();
					todo.pop();

					void* iter = nullptr;
					do
					{
						auto rawNestedClass = mono_class_get_nested_types(curNestedClass->getInternalClass(), &iter);
						if (rawNestedClass == nullptr)
							break;
						auto nestedType = curNestedClass->getTypeName() + "+" + mono_class_get_name(rawNestedClass);
						auto nestedClass = getClass(ns, nestedType, rawNestedClass);
						if (nestedClass != nullptr)
						{
							// Skip compiler generated classes
							if (nestedClass->hasAttribute(compilerGeneratedAttrib))
								continue;

							cachedClassList.push_back(nestedClass);
							todo.push(nestedClass);
						}

					} while (true);
				}

				cachedClassList.push_back(curClass);
			}
		}

		haveCachedClassList = true;

		return cachedClassList;
	}

	auto MapleMonoAssembly::invoke(const std::string& functionName) -> void
	{
		MonoMethodDesc* methodDesc = mono_method_desc_new(functionName.c_str(), false);
		if (methodDesc != nullptr)
		{
			::MonoMethod* entry = mono_method_desc_search_in_image(methodDesc, monoImage);

			if (entry != nullptr)
			{
				MonoObject* exception = nullptr;
				mono_runtime_invoke(entry, nullptr, nullptr, &exception);

				MonoHelper::throwIfException(exception);
			}
		}
	}

	auto MapleMonoAssembly::load() -> void
	{

		if (loaded)
			unload();

		File file(path);
		auto buffer = file.getBuffer();
		auto imageName = StringUtils::getFileName(path);


		MonoImageOpenStatus status = MONO_IMAGE_OK;
		MonoImage* image = mono_image_open_from_data_with_name((char*)buffer.get(), file.getFileSize(), true, &status, false, imageName.c_str());

		if (status != MONO_IMAGE_OK || image == nullptr)
		{
			LOGE("Failed loading image data for assembly {0} ", path);
			return;
		}

		// Load MDB file
#if MAPLE_DEBUG
		auto mdbPath = mPath + ".mdb";
		if (File::fileExists(mdbPath))
		{
			File file(mdbPath);
			auto buffer = file.getBuffer();

			if (file)
			{

				mono_debug_open_image_from_memory(image, debugData, mdbSize);
			}
		}
#endif

		monoAssembly = mono_assembly_load_from_full(image, imageName.c_str(), &status, false);
		if (status != MONO_IMAGE_OK || monoAssembly == nullptr)
		{
			LOGE("Failed loading assembly {0}", path);
			return;
		}

		monoImage = image;
		if (monoImage == nullptr)
		{
			LOGE("Cannot get script assembly image.");
		}
		loaded = true;
		dependency = false;
	}


	auto MapleMonoAssembly::loadFromImage(MonoImage* image) -> void
	{
		auto monoAssembly = mono_image_get_assembly(image);
		if (monoAssembly == nullptr)
		{
			LOGE("Cannot get assembly from image.");
		}

		monoAssembly = monoAssembly;
		monoImage = image;
		loaded = true;
		dependency = true;
	}
	auto MapleMonoAssembly::unload() -> void
	{
		if (!loaded)
			return;

		
		classes.clear();
		classesByRaw.clear();
		cachedClassList.clear();
		haveCachedClassList = false;

		if (!dependency)
		{
			if (debugData != nullptr)
			{
				mono_debug_close_image(monoImage);

				delete debugData;
				debugData = nullptr;
			}

			if (monoImage != nullptr)
			{
				mono_image_close(monoImage);
				monoImage = nullptr;
			}

			monoAssembly = nullptr;
			loaded = false;
		}
	}
	auto MapleMonoAssembly::isGenericClass(const std::string& name) const -> bool
	{
		// By CIL convention generic classes have ` separating their name and
		// number of generic parameters
		auto iterFind = std::find(name.rbegin(), name.rend(), '`');
		return iterFind != name.rend();
	}

	size_t MapleMonoAssembly::ClassId::Hash::operator()(const ClassId& v) const
	{
		size_t genInstanceAddr = (size_t)v.genericInstance;

		size_t seed = 0;
		HashCode::hashCode(seed, v.namespaceName, v.name, genInstanceAddr);
		return seed;
	}

	MapleMonoAssembly::ClassId::ClassId(const std::string& namespaceName, std::string name, MonoClass* genericInstance /*= nullptr*/)
		:namespaceName(namespaceName), name(name), genericInstance(genericInstance)
	{

	}

};