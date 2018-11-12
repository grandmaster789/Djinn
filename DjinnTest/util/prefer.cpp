#include "stdafx.h"
#include "CppUnitTest.h"

#include "util/algorithm.h"
#include <sstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace DjinnTest {
    TEST_CLASS(Prefer) {
    public:
        TEST_METHOD(prefer_enumclass) {
            {
                enum class eVegetables {
                    asparagus,
                    broccoli,
                    cauliflower,
                    daikon,
                    eggplant
                };

                std::vector<eVegetables> available_ingredients = {
                    eVegetables::broccoli,
                    eVegetables::eggplant,
                    eVegetables::cauliflower
                };

                auto selected_ingredient = djinn::util::prefer(
                    available_ingredients,
                    eVegetables::asparagus,
                    eVegetables::daikon
                );

                Assert::IsTrue(!selected_ingredient.has_value());

                selected_ingredient = djinn::util::prefer(
                    available_ingredients,
                    eVegetables::daikon,
                    eVegetables::cauliflower
                );
                Assert::IsTrue(*selected_ingredient == eVegetables::cauliflower);
            }
        }
    };
}