#include "log.h"
#include <filesystem>

ecs_logger::ecs_logger(const char* n) : modname(n){
#if _LOG_USE_FILE == 1
	up();
#endif
}

ecs_logger::~ecs_logger() {
#if _LOG_USE_FILE == 1
	down();
#endif
}
