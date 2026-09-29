#include "task_scheduler.hpp"
#include "../../cheat/initialise.hpp"
#include "../offsets.hpp"
#include "../../utils/output/output.hpp"
uintptr_t task_scheduler::get_scheduler() {
    auto module = memory->get_module_address();
    uintptr_t result = memory->read<uintptr_t>(module + Offsets::TaskScheduler::Pointer);
    logger::print<logger::level::info>("TaskScheduler pointer resolved: " + (result));
    return result;
}

std::vector<rbx::instance_t> task_scheduler::get_jobs() {
    std::vector<rbx::instance_t> jobs;
    uintptr_t scheduler = get_scheduler();

    uintptr_t jobStart = memory->read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobStart);
    uintptr_t jobEnd = memory->read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobEnd);

    logger::print<logger::level::info>("JobStart: 0x" + (jobStart));
    logger::print<logger::level::info>("JobEnd: 0x" + (jobEnd));

    if (jobStart && jobEnd && jobStart < jobEnd) {
        for (uintptr_t job = jobStart; job < jobEnd; job += 0x10) {
            uintptr_t jobAddress = memory->read<uintptr_t>(job);
            if (jobAddress) {
                jobs.emplace_back(jobAddress);
            }
        }
    }

    return jobs;
}

uintptr_t task_scheduler::get_job_by_name(const std::string& targetName) {
    uintptr_t scheduler = get_scheduler();
    uintptr_t jobStart = memory->read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobStart);
    uintptr_t jobEnd = memory->read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobEnd);
    for (uintptr_t i = jobStart; i < jobEnd; i += 0x10) {
        uintptr_t jobAddr = memory->read<uintptr_t>(i);
        if (!jobAddr) continue;

        std::string jobName = memory->read_string(jobAddr + Offsets::TaskScheduler::JobName);
        if (jobName.find(targetName) != std::string::npos) {
            return jobAddr;
        }
    }

    return 0;
}


uintptr_t task_scheduler::get_target_fps() {
    uintptr_t scheduler = get_scheduler();
    double rawFps = memory->read<double>(scheduler + Offsets::TaskScheduler::MaxFPS);
    return static_cast<uintptr_t>(1.0 / rawFps);
}

uintptr_t task_scheduler::set_target_fps(double value) {
    uintptr_t scheduler = get_scheduler();
    double toWrite = 1.0 / value;
    memory->write<double>(scheduler + Offsets::TaskScheduler::MaxFPS, toWrite);
    return scheduler;
}
