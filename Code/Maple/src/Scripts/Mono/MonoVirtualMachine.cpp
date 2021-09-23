
#include "MonoVirtualMachine.h"
#include "Mono.h"
#include "MonoExporter.h"
#include "MonoHelper.h"
#include "MapleMonoClass.h"
#include "MapleMonoAssembly.h"
#include "MonoScriptInstance.h"

#include "Others/StringUtils.h"
#include "Others/Console.h"

#include "FileSystem/File.h"
#include "Application.h"

namespace Maple
{
	void monoLogCallback(const char* logDomain, const char* logLevel, const char* message, mono_bool fatal, void* userData)
	{
		static const char* monoErrorLevels[] =
		{
			nullptr,
			"error",
			"critical",
			"warning",
			"message",
			"info",
			"debug"
		};

		uint32_t errorLevel = 0;
		if (logLevel != nullptr)
		{
			for (uint32_t i = 1; i < 7; i++)
			{
				if (strcmp(monoErrorLevels[i], logLevel) == 0)
				{
					errorLevel = i;
					break;
				}
			}
		}

		if (errorLevel == 0)
		{
			LOGE("Mono: {0} in domain {1}", message, logDomain);
		}
		else if (errorLevel <= 2)
		{
			LOGE("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else if (errorLevel <= 3)
		{
			LOGW("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else
		{
			LOGV("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
	}

	void monoPrintCallback(const char* string, mono_bool isStdout)
	{
		LOGW("Mono error: {0}", string);
	}

	void monoPrintErrorCallback(const char* string, mono_bool isStdout)
	{
		LOGE("Mono error: {0}", string);
	}


	MonoVirtualMachine::MonoVirtualMachine()
	{
		
	}

	MonoVirtualMachine::~MonoVirtualMachine()
	{
		assemblies.clear();
		unloadScriptDomain();
		if (domain != nullptr) {
			mono_jit_cleanup(domain);
		}
		scriptInstances.clear();
	}


	auto MonoVirtualMachine::init() -> void
	{
		// Get file paths
		const std::string dir_mono_lib = "../Toolchains/mono/lib";
		const std::string dir_mono_etc = "../Toolchains/mono/etc";
		// Point mono to the libs and configuration files
		mono_set_dirs(dir_mono_lib.c_str(), dir_mono_etc.c_str());

		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		// GC options:
		// check-remset-consistency: Makes sure that write barriers are properly issued in native code, and therefore
		//    all old->new generation references are properly present in the remset. This is easy to mess up in native
		//    code by performing a simple memory copy without a barrier, so it's important to keep the option on.
		// verify-before-collections: Unusure what exactly it does, but it sounds like it could help track down
		//    things like accessing released/moved objects, or attempting to release handles for an unloaded domain.
		// xdomain-checks: Makes sure that no references are left when a domain is unloaded.

		const char* options[] = {
			"--soft-breakpoints",
			"--debugger-agent=transport=dt_socket,address=127.0.0.1:17615,embedding=1,server=y,suspend=n",
			"--debug-domain-unload",
			"--gc-debug=check-remset-consistency,verify-before-collections,xdomain-checks"
		};
		mono_jit_parse_options(4, (char**)options);
		
		mono_trace_set_log_handler(monoLogCallback, this);
		mono_trace_set_print_handler(monoPrintCallback);
		mono_trace_set_printerr_handler(monoPrintErrorCallback);

		domain = mono_jit_init_version("MapleEngine", "v4.0.30319");
		if (!domain)
		{
			LOGE("mono_jit_init failed");
			return;
		}

		if (!mono_domain_set((MonoDomain*)domain, false))
		{
			LOGE("mono_domain_set failed");
			return;
		}
		// soft debugger needs this
		mono_thread_set_main(mono_thread_current());
		MonoExporter::exportMono();

		corlibAssembly = std::make_shared<MapleMonoAssembly>("corlib", "corlib");
		corlibAssembly->loadFromImage(mono_get_corlib());
		assemblies["corlib"] = corlibAssembly;

		LOGI("Mono Init over");
	}


	auto MonoVirtualMachine::findClass(const std::string& ns, const std::string& type) -> std::shared_ptr<MapleMonoClass>
	{
		std::shared_ptr<MapleMonoClass> monoClass = nullptr;
		for (auto& assembly : assemblies)
		{
			monoClass = assembly.second->getClass(ns, type);
			if (monoClass != nullptr)
				return monoClass;
		}

		return nullptr;
	}

	auto MonoVirtualMachine::findClass(MonoClass* clazz) -> std::shared_ptr<MapleMonoClass>
	{
		std::shared_ptr<MapleMonoClass> monoClass;
		for (auto& assembly : assemblies)
		{
			monoClass = assembly.second->getClass(clazz);
			if (monoClass != nullptr)
				return monoClass;
		}
		return nullptr;
	}

	auto MonoVirtualMachine::get() ->std::shared_ptr<MonoVirtualMachine>
	{
		return Application::get()->getMonoVm();
	}

	auto MonoVirtualMachine::loadAssembly(const std::string& path, const std::string& name) -> std::shared_ptr<MapleMonoAssembly>
	{
		if (scriptDomain == nullptr)
		{
			char * appDomainName = "ScriptDomain";

			scriptDomain = mono_domain_create_appdomain(appDomainName, nullptr);
			if (scriptDomain == nullptr) {
				throw std::runtime_error("Cannot create script app domain.");
			}

			if (!mono_domain_set(scriptDomain, true)) {
				throw std::runtime_error("Cannot set script app domain.");
			}
		}
		std::shared_ptr<MapleMonoAssembly> assembly;
		auto iterFind = assemblies.find(name);
		if (iterFind != assemblies.end())
		{
			assembly = iterFind->second;
		}
		else
		{
			assembly = std::make_shared<MapleMonoAssembly>(path, name);
			assemblies[name] = assembly;
		}

		initializeAssembly(assembly);
		LOGV("loadAssembly...");
		return assembly;
	}

	auto MonoVirtualMachine::initializeAssembly(std::shared_ptr<MapleMonoAssembly> assembly) -> void
	{
		if (!assembly->loaded)
		{
			assembly->load();

			// Fully initialize all types that use this assembly
			auto & typeMetas = scriptInstances[assembly->name];
			for (auto& meta : typeMetas)
			{
				meta->scriptClass = assembly->getClass(meta->ns, meta->name);
				if (meta->scriptClass == nullptr)
				{
					LOGE("Unable to find class of type: {0}::{1}",meta->ns,meta->name);
					throw std::runtime_error("Unable to find class");
				}

				if (meta->scriptClass->hasField("mCachedPtr"))
					meta->thisPtrField = meta->scriptClass->getField("mCachedPtr");
				else
					meta->thisPtrField = nullptr;

				meta->initCallback();
			}
		}
	}

	auto MonoVirtualMachine::unloadScriptDomain() -> void
	{
		if (scriptDomain != nullptr)
		{
			mono_domain_set(mono_get_root_domain(), true);
			MonoObject* exception = nullptr;
			mono_domain_try_unload(scriptDomain, &exception);

			if (exception != nullptr)
				MonoHelper::throwIfException(exception);

			scriptDomain = nullptr;
		}

		for (auto& assemblyEntry : assemblies)
		{
			assemblyEntry.second->unload();

			auto & typeMetas = scriptInstances[assemblyEntry.first];
			for (auto& entry : typeMetas)
			{
				entry->scriptClass = nullptr;
				entry->thisPtrField = nullptr;
			}
		}

		assemblies.clear();
		assemblies["corlib"] = corlibAssembly;
	}

	auto MonoVirtualMachine::compileAssembly(const std::function<void(void*)>& callback) -> void
	{
		LOGV("compileAssembly...");
		unloadScriptDomain();
		Application::get()->getThreadPool()->addTask([=]() -> void*{
			std::vector<std::string> out;
			File::list(out, [](const std::string& str) -> bool {
				return StringUtils::endWith(str, ".cs");
			});
			MonoHelper::compileScript(out, "MapleLibrary.dll");
			auto event = std::make_unique<RecompileScriptsEvent>();
			event->scene = Application::get()->getSceneManager()->getCurrentScene();
			Application::get()->getEventDispatcher().postEvent(std::move(event));
			return nullptr;
		}, callback);
	}
		

};