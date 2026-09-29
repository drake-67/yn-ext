#pragma once
#include "../../../sdk/datamodel/instance_t.hpp"
#include "../../../sdk/datamodel/players.hpp"
#include "../../../sdk/datamodel/datamodel.hpp"

namespace player_cache {
    void hook_cache();
}

namespace rbx {
    class player_t {
    public:
        bool localplayer = false;
        rbx::instance_t team;
        bool r6 = true;
        rbx::instance_t tool;
        std::string ToolName;
        float Health;
        float MaxHealth;

        float distance = 0.f; // ⬅️ Distance to local player (in studs)

        std::string Name;
        std::vector<rbx::instance_t> children;
        std::vector<rbx::instance_t> bones;
        rbx::instance_t player;
        rbx::instance_t character;
        std::uint64_t userid;
        rbx::instance_t head;
        rbx::instance_t hrp;
        rbx::instance_t humanoid;

        rbx::instance_t leftarm;
        rbx::instance_t rightarm;

        rbx::instance_t lefthand;
        rbx::instance_t righthand;
        rbx::instance_t leftlowerarm;
        rbx::instance_t rightlowerarm;
        rbx::instance_t leftupperarm;
        rbx::instance_t rightupperarm;

        rbx::instance_t leftfoot;
        rbx::instance_t rightfoot;
        rbx::instance_t rightupperleg;
        rbx::instance_t leftupperleg;
        rbx::instance_t leftlowerleg;
        rbx::instance_t rightlowerleg;

        rbx::instance_t uppertorso;
        rbx::instance_t lowertorso;
        rbx::instance_t torso;

        rbx::instance_t leftleg;
        rbx::instance_t rightleg;

        bool operator==(rbx::player_t& other) {
            return character.address == other.character.address;
        }
    };
}
