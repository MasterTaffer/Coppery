#pragma once
#include <vector>
#include <string>

std::vector<std::string> SplitStringByComma(const std::string& str);
std::vector<std::string> SplitStringByComma(const char* str);
bool IsPresentInList(const std::vector<std::string>& haystack, const char* needle);