#include "string_util.h"
#include <functional>
#include <locale>
#include <algorithm>

namespace djinn::util {
    std::string concat(
        const std::vector<std::string>& parts, 
        const std::string& separator
    ) {
        std::string result;
    
        for (const auto& part : parts) {
            result.append(part);
            result.append(separator);
        }
    
        return result;
    }
    
    std::string concat(
        const std::vector<const char*>& parts,
        const std::string& separator
    ) {
        std::string result;
    
        for (const auto& part : parts) {
            result.append(part);
            result.append(separator);
        }
    
        return result;
    }
    
    std::vector<std::string> split(
        const std::string& source,
        const char separator
    ) {
        using namespace std;
        
        vector<string> result;
    
        size_t cursor;
        size_t last_position = 0;
        size_t len           = source.length();
    
        while (last_position < len + 1) {
            cursor = source.find(separator, last_position);
    
            if (cursor == string::npos)
                cursor = len;
    
            if (cursor != last_position)
                result.push_back(
                    string(
                        source.begin() + last_position, 
                        source.begin() + cursor
                    )
                );
    
            last_position = cursor + 1;
        }
    
        return result;
    }
    
    std::vector<std::string> split(
        const std::string& source,
        const std::string& delimiter
    ) {
        using namespace std;
    
        if (delimiter.size() == 0)
            return { source };
    
        if (delimiter.size() == 1)
            return split(source, delimiter[0]);
    
        vector<string> result;
    
        size_t cursor;
        size_t last_position = 0;
        size_t len           = source.length();
    
        while (last_position < len + 1) {
            // for some reason, MSVC requires an explicit template parameter for boyer_moore_searcher...
            auto it = search(
                source.begin() + last_position,
                source.end(),
                boyer_moore_horspool_searcher<string::const_iterator>( 
                    delimiter.begin(),
                    delimiter.end()
                )
            );
    
            cursor = distance(source.begin(), it);
    
            if (cursor == string::npos)
                cursor = len;
    
            if (cursor != last_position)
                result.push_back(
                    string(
                        source.begin() + last_position,
                        source.begin() + cursor
                    )
                );
    
            last_position = cursor + delimiter.size();
        }
    
        return result;
    }
    
    std::vector<std::string> split(
        const std::string& source,
        const std::vector<std::string>& delimiters
    ) {
        using namespace std;
    
        if (delimiters.empty())
            return { source };
    
        if (delimiters.size() == 1)
            return split(source, delimiters[0]);
    
        vector<string> result;
    
        size_t cursor;
        size_t last_position = 0;
        size_t delim_size    = 0;
        size_t len           = source.length();
    
        while (last_position < len + 1) {
            // start each loop by placing the cursor at the end
            cursor = len; 
    
            // check each delimiter, and select the one with the minimum distance
            for (const auto& delim: delimiters) {
                auto it = search(
                    source.begin() + last_position,
                    source.end(),
                    boyer_moore_horspool_searcher<string::const_iterator>(
                        delim.begin(),
                        delim.end()
                    )
                );
    
                auto d = static_cast<size_t>(distance(source.begin(), it));
    
                if (d < cursor) {
                    cursor = d;
                    delim_size = delim.size();
                }
            }
    
            if (cursor == string::npos)
                cursor = len;
    
            if (cursor != last_position)
                result.push_back(
                    string(
                        source.begin() + last_position,
                        source.begin() + cursor
                    )
                );
    
            last_position = cursor + delim_size;
        }
    
        return result;
    }
    
    std::string toUpper(const std::string& s) {
        std::string result = s;
    
        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char c) { 
                return std::toupper(c, std::locale()); 
            }
        );
    
        return result;
    }
    
    std::string toLower(const std::string& s) {
        std::string result = s;
    
        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char c) { 
                return std::tolower(c, std::locale()); 
            }
        );
    
        return result;
    }

    std::string columnize(
        const std::string& source_string, 
        const int          column_width, 
        const char         separator
    ) {
        std::string result;
        result.reserve(source_string.size() + source_string.size() / column_width);

        int cursor = 0;

        for (const auto& character : source_string) {
            result.push_back(character);

            if (cursor > column_width) {
                cursor = 0;
                result.push_back(separator);
            }
        }

        return result;
    }
}