#pragma once
#include <cstdint>
#include "../../sdk/datamodel/instance_t.hpp"

class task_scheduler final {
public:
    std::uint64_t address = 0;
    static uintptr_t get_scheduler();
    std::vector<rbx::instance_t> get_jobs();
    uintptr_t get_job_by_name(const std::string& targetName);
    uintptr_t get_target_fps();
    uintptr_t set_target_fps(double value);


private:

};

