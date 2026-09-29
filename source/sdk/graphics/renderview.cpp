#include "renderview.hpp"
#include "../schedule/task_scheduler.hpp"
#include "../../cheat/initialise.hpp"


namespace rbx {

	std::uint64_t renderview_t::get_render_view() {
		return memory->read<uintptr_t>(task_scheduler{}.get_job_by_name("RenderJob") + 0x218);
	}
}
