#pragma once
#include <stdexcept>
#include <string.h>

namespace except {
	inline std::string getErrStr(int errNumber) {
		char buffer[512];
		strerror_s(buffer, errNumber);
		return std::string(buffer, 512);
	}

	struct fileOpenFail_ex : std::runtime_error {
		inline fileOpenFail_ex(const std::string& fileName, int reason) : std::runtime_error("Failed to open file " + fileName + ", " + getErrStr(reason)) {}
	};

	struct config_ex : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
	struct configValueMissing_ex : config_ex {
		inline configValueMissing_ex(const std::string& valueName) : config_ex("Missing value: " + valueName) {}
	};
	struct configValueMistyped_ex : config_ex {
		inline configValueMistyped_ex(const std::string& valueName, const std::string& expected) : config_ex("Value " + valueName + " has incorrect type, expected " + expected) {}
	};
	struct configValueInvalid_ex : config_ex {
		inline configValueInvalid_ex(const std::string& valueName, const std::string& reason) : config_ex("Value " + valueName + " is invalid, " + reason) {}
	};

	struct objectOccupied_ex : std::runtime_error {
		using std::runtime_error::runtime_error;
	};
}