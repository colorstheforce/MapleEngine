

#include "Console.h"
#include "Scripts/LuaVirtualMachine.h"
extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}
#include <LuaBridge/LuaBridge.h>

namespace Maple 
{
	auto LogExport::exportLua(lua_State* L) -> void
	{
		luabridge::getGlobalNamespace(L)
			.addFunction("LOGE", std::function <void(const std::string&)>([](const std::string& str) {
			LOGE(str);
				}))
			.addFunction("LOGV", std::function <void(const std::string&)>([](const std::string& str) {
					LOGV(str);
				}))
					.addFunction("LOGI", std::function <void(const std::string&)>([](const std::string& str) {
					LOGI(str);
						}))
					.addFunction("LOGC", std::function <void(const std::string&)>([](const std::string& str) {
							LOGC(str);
						}));
	}
};
