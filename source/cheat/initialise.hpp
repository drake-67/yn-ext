#pragma once
#include <memory>
#include "../utils/memory/memory.h"
// placeholder 
inline std::unique_ptr<memory_t> memory = std::make_unique<memory_t>();

namespace initialise {
	void setup();
}

namespace renderer_t {
	void start();
}

namespace teleport_handler_t {
   void update(); 
}

