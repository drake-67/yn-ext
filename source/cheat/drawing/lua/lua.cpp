#define _CRT_SECURE_NO_WARNINGS
#define l_likely(x) (x)
#define l_unlikely(x) (x)

#include "luau.hpp"
#include <sstream>

#include "lua_files/lualib.h"  // <-- Including a header
#include "lua_files/lauxlib.c"
#pragma comment(lib, "lua54.lib")

namespace LuaVM {

    Manager* g_manager = nullptr;

    Manager::Manager() : L(nullptr), maxRoutines(50) {}

    Manager::~Manager() {
        shutdown();
    }

    void Manager::initialize() {
        L = luaL_newstate();
       //   luaL_openlibs(L);   
        // will change it later hardcoded. not lua 

        registerRoutineFunctions();
        registerEnvironmentFunctions();
        g_manager = this;
    }

    void Manager::shutdown() {
        std::lock_guard<std::mutex> lock(routineMutex);

        for (auto& pair : routines) {
            if (pair.second->nativeThread) {
                pair.second->running = false;
                if (pair.second->nativeThread->joinable()) {
                    pair.second->nativeThread->join();
                }
                delete pair.second->nativeThread;
            }
            if (pair.second->thread) {
                lua_close(pair.second->thread);
            }
            delete pair.second;
        }
        routines.clear();

        if (L) {
            lua_close(L);
            L = nullptr;
        }
    }

    std::string Manager::execute(const std::string& code) {
        if (luaL_loadstring(L, code.c_str()) != LUA_OK) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return "Error: " + error;
        }

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            std::string error = lua_tostring(L, -1);
            lua_pop(L, 1);
            return "Error: " + error;
        }

        return "Success";
    }

    void Manager::registerRoutineFunctions() {
        lua_newtable(L);

        lua_pushcfunction(L, routine_create);
        lua_setfield(L, -2, "create");

        lua_pushcfunction(L, routine_destroy);
        lua_setfield(L, -2, "destroy");

        lua_pushcfunction(L, routine_running);
        lua_setfield(L, -2, "running");

        lua_pushcfunction(L, routine_list);
        lua_setfield(L, -2, "list");

        lua_pushcfunction(L, routine_wait);
        lua_setfield(L, -2, "wait");

        lua_pushcfunction(L, routine_suspend);
        lua_setfield(L, -2, "suspend");

        lua_pushcfunction(L, routine_suspendall);
        lua_setfield(L, -2, "suspendall");

        lua_pushcfunction(L, routine_resume);
        lua_setfield(L, -2, "resume");

        lua_setglobal(L, "routine");
    }

    void Manager::registerEnvironmentFunctions() {
        lua_pushcfunction(L, env_getgenv);
        lua_setglobal(L, "getgenv");

        lua_pushcfunction(L, env_getfenv);
        lua_setglobal(L, "getfenv");

        lua_pushcfunction(L, env_setfenv);
        lua_setglobal(L, "setfenv");

        lua_pushcfunction(L, env_touserdata);
        lua_setglobal(L, "touserdata");

        lua_pushcfunction(L, env_execute);
        lua_setglobal(L, "execute");

        lua_pushcfunction(L, env_loadstring);
        lua_setglobal(L, "loadstring");
    }

    int Manager::routine_create(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        std::lock_guard<std::mutex> lock(g_manager->getMutex());

        if (g_manager->getRoutines().size() >= g_manager->maxRoutines) {
            return luaL_error(L, "Maximum routine limit reached");
        }

        if (g_manager->getRoutines().find(name) != g_manager->getRoutines().end()) {
            return luaL_error(L, "Routine already exists: %s", name);
        }

        Routine* routine = new Routine();
        routine->name = name;
        routine->thread = lua_newthread(L);
        routine->suspended = false;
        routine->running = true;

        lua_pushvalue(L, 2);
        lua_xmove(L, routine->thread, 1);

        routine->nativeThread = new std::thread([routine]() {
            while (routine->running) {
                if (!routine->suspended) {
                    if (lua_pcall(routine->thread, 0, 0, 0) != LUA_OK) {
                        const char* error = lua_tostring(routine->thread, -1);
                        lua_pop(routine->thread, 1);
                        routine->running = false;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            });

        g_manager->getRoutines()[name] = routine;
        return 0;
    }

    int Manager::routine_destroy(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        g_manager->destroyRoutine(name);
        return 0;
    }

    int Manager::routine_running(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        bool running = g_manager->isRoutineRunning(name);
        lua_pushboolean(L, running);
        return 1;
    }

    int Manager::routine_list(lua_State* L) {
        auto list = g_manager->getRoutineList();
        lua_newtable(L);
        for (size_t i = 0; i < list.size(); i++) {
            lua_pushstring(L, list[i].c_str());
            lua_rawseti(L, -2, i + 1);
        }
        return 1;
    }

    int Manager::routine_wait(lua_State* L) {
        double seconds = luaL_checknumber(L, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000)));
        return 0;
    }

    int Manager::routine_suspend(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        g_manager->suspendRoutine(name);
        return 0;
    }

    int Manager::routine_suspendall(lua_State* L) {
        g_manager->suspendAllRoutines();
        return 0;
    }

    int Manager::routine_resume(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        g_manager->resumeRoutine(name);
        return 0;
    }

    int Manager::env_getgenv(lua_State* L) {
        lua_pushglobaltable(L);
        return 1;
    }

    int Manager::env_getfenv(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        lua_getupvalue(L, 1, 1);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_pushglobaltable(L);
        }
        return 1;
    }

    int Manager::env_setfenv(lua_State* L) {
        luaL_checktype(L, 1, LUA_TFUNCTION);
        luaL_checktype(L, 2, LUA_TTABLE);
        lua_pushvalue(L, 2);
        const char* name = lua_setupvalue(L, 1, 1);
        lua_pushboolean(L, name != NULL);
        return 1;
    }

    int Manager::env_touserdata(lua_State* L) {
        lua_Number num = luaL_checknumber(L, 1);
        lua_pushlightuserdata(L, (void*)(uintptr_t)num);
        return 1;
    }

    int Manager::env_execute(lua_State* L) {
        const char* code = luaL_checkstring(L, 1);

        if (luaL_loadstring(L, code) != LUA_OK) {
            return lua_error(L);
        }

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            return lua_error(L);
        }

        return 0;
    }

    int Manager::env_loadstring(lua_State* L) {
        return env_execute(L);
    }

    bool Manager::createRoutine(const std::string& name, const std::string& code) {
        std::lock_guard<std::mutex> lock(routineMutex);

        if (routines.size() >= maxRoutines || routines.find(name) != routines.end()) {
            return false;
        }

        Routine* routine = new Routine();
        routine->name = name;
        routine->thread = lua_newthread(L);
        routine->suspended = false;
        routine->running = true;

        if (luaL_loadstring(routine->thread, code.c_str()) != LUA_OK) {
            lua_pop(L, 1);
            delete routine;
            return false;
        }

        routine->nativeThread = new std::thread([routine]() {
            while (routine->running) {
                if (!routine->suspended) {
                    if (lua_pcall(routine->thread, 0, 0, 0) != LUA_OK) {
                        lua_pop(routine->thread, 1);
                        routine->running = false;
                        break;
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            });

        routines[name] = routine;
        return true;
    }

    bool Manager::destroyRoutine(const std::string& name) {
        std::lock_guard<std::mutex> lock(routineMutex);

        auto it = routines.find(name);
        if (it == routines.end()) return false;

        Routine* routine = it->second;
        routine->running = false;

        if (routine->nativeThread && routine->nativeThread->joinable()) {
            routine->nativeThread->join();
        }

        delete routine->nativeThread;
        delete routine;
        routines.erase(it);
        return true;
    }

    bool Manager::suspendRoutine(const std::string& name) {
        std::lock_guard<std::mutex> lock(routineMutex);

        auto it = routines.find(name);
        if (it == routines.end()) return false;

        it->second->suspended = true;
        return true;
    }

    bool Manager::resumeRoutine(const std::string& name) {
        std::lock_guard<std::mutex> lock(routineMutex);

        auto it = routines.find(name);
        if (it == routines.end()) return false;

        it->second->suspended = false;
        return true;
    }

    bool Manager::isRoutineRunning(const std::string& name) {
        std::lock_guard<std::mutex> lock(routineMutex);

        auto it = routines.find(name);
        if (it == routines.end()) return false;

        return it->second->running && !it->second->suspended;
    }

    void Manager::suspendAllRoutines() {
        std::lock_guard<std::mutex> lock(routineMutex);

        for (auto& pair : routines) {
            pair.second->suspended = true;
        }
    }

    std::vector<std::string> Manager::getRoutineList() {
        std::lock_guard<std::mutex> lock(routineMutex);

        std::vector<std::string> list;
        for (const auto& pair : routines) {
            list.push_back(pair.first);
        }
        return list;
    }
}