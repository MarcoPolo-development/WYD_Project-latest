#pragma once
#include "pch.h"

namespace logg {

	void error(char* msg) {
		std::cout << hue::red << "[Error]" << *msg << std::endl;
	}

	void info(char* msg) {
		std::cout << hue::aqua << "[Info]" << *msg << std::endl;
	}

	void warning(char* msg) {
		std::cout << hue::yellow << "[Warn]" << *msg << std::endl;
	}

	void hexinfo(char* msg) {
		std::cout << std::hex << hue::aqua << "[Info]" << *msg << std::dec << std::endl;
	}
}