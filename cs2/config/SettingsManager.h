#pragma once

#include <string>

#include <iostream>

class SettingsManager
{
public:
	std::string language = "ch";

	void LoadSettings();
};

inline SettingsManager settingsJson;
