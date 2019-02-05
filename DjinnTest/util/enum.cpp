#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/algorithm.h"
#include <sstream>
#include <vector>
#include <map>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace djinn;

namespace DjinnTest {
    TEST_CLASS(Enum) {
    public:
		TEST_METHOD(enum_iterator) {
			std::vector<char> poor_string = {
				'a', 'b', 'c', 'd', 'e', 'f'
			};

			std::stringstream sstr;

			for (auto[idx, val] : util::enumerate(poor_string))
				sstr << idx << ":" << val << " ";

			Assert::IsTrue(sstr.str() == "0:a 1:b 2:c 3:d 4:e 5:f ");
		}
    };
}