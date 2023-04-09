#pragma once
#include <Windows.h>
#include <string>
class Utils
{
public:

	
	static std::string GetFullPathTo(std::string relativeFilePath);
	static std::wstring GetFullPathTo_Wide(std::wstring relativeFilePath);

private:
	// Helpers for determining the actual path to the executable
	static std::string GetExePath();
	static std::wstring GetExePath_Wide();
};

