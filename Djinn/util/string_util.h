#pragma once

#include <string>
#include <vector>

// [NOTE] everything here should probably have overloads for wstring as well... don't really feel like going into the
// whole
//        unicode vs wchar_t thing, as it quickly leads into linux vs windows which is a minefield on its own. Text is
//        hard.
// [NOTE] if I ever find a bottleneck wrt strings it may be good to look into (absl) or (folly) string methods

namespace djinn::util {
	std::string
	    concat(const std::vector<std::string>& parts, const std::string& separator = "");

	std::string
	    concat(const std::vector<const char*>& parts, const std::string& separator = "");

	// [NOTE] the split functions do not trim empty substrings
	// [NOTE] per character is reasonable, using string delimiters is more complicated, a set of possible delimiter
	// strings is complex [NOTE] there are even more variations to specify splitting; add as required...
	std::vector<std::string> split(const std::string& source, const char separator = '\n');

	// [NOTE] this is currently using boyer-moore-horspool, alternatives are certainly possible
	std::vector<std::string>
	    split(const std::string& source, const std::string& separator = "\n");

	// [NOTE] this is a bit difficult to use really, probably not worth having in here
	std::vector<std::string>
	    split(const std::string& source, const std::vector<std::string>& separators);

	std::string toUpper(const std::string& s);
	std::string toLower(const std::string& s);

	template <typename... tArgs>
	std::string stringify(tArgs&&... args);

	// insert a 'separator' character every 'column_width' characters
	std::string columnize(
	    const std::string& source_string,
	    const int          column_width = 80,
	    const char         separator    = '\n');
}  // namespace djinn::util

#include "string_util.inl"
