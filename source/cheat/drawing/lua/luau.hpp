#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define l_likely(x) (x)
#define l_unlikely(x) (x)

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif
#include "lua_files/lua.h"



#ifdef __cplusplus
}
#endif

namespace LuaVM {

    struct Routine {
        std::string name;
        lua_State* thread;
        std::thread* nativeThread;
        bool suspended;
        bool running;
    };

    class Manager {
    private:
        lua_State* L;
        std::map<std::string, Routine*> routines;
        std::mutex routineMutex;
        int maxRoutines;

        static int routine_create(lua_State* L);
        static int routine_destroy(lua_State* L);
        static int routine_running(lua_State* L);
        static int routine_list(lua_State* L);
        static int routine_wait(lua_State* L);
        static int routine_suspend(lua_State* L);
        static int routine_suspendall(lua_State* L);
        static int routine_resume(lua_State* L);

        static int env_getgenv(lua_State* L);
        static int env_getfenv(lua_State* L);
        static int env_setfenv(lua_State* L);
        static int env_touserdata(lua_State* L);
        static int env_execute(lua_State* L);
        static int env_loadstring(lua_State* L);

        void registerRoutineFunctions();
        void registerEnvironmentFunctions();

    public:
        Manager();
        ~Manager();

        void initialize();
        void shutdown();
        std::string execute(const std::string& code);

        bool createRoutine(const std::string& name, const std::string& code);
        bool destroyRoutine(const std::string& name);
        bool suspendRoutine(const std::string& name);
        bool resumeRoutine(const std::string& name);
        bool isRoutineRunning(const std::string& name);
        void suspendAllRoutines();
        std::vector<std::string> getRoutineList();

        lua_State* getState() { return L; }
        std::map<std::string, Routine*>& getRoutines() { return routines; }
        std::mutex& getMutex() { return routineMutex; }
    };

    extern Manager* g_manager;
}