#pragma once

#include <string>
#include <vector>
#include <atomic>

enum class AppState {
	DMA_INITIALIZING,
	DMA_FAILED,
	SEARCHING_GAME,
	INITIALIZING_GAME,
	RUNNING,
};

namespace globalVars {
	inline float windowx = 800;
	inline float windowy = 500;
	inline std::atomic<AppState> gameState{ AppState::DMA_INITIALIZING };
}

namespace Keys {
	inline bool MenuKey = false;
	inline bool RecordKey = false;  // For grenade position recording
}
