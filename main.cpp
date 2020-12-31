#include "function_maxima.h"

#include "iostream"

int main() {
    FunctionMaxima<int, int> F;

    // check sets
    F.set_value(5, 1);
    F.set_value(6, 2);
    F.set_value(4, 3);
    F.set_value(3, 4);
    F.set_value(2, 5);

    // check find
    std::cout << F.value_at(3) << std::endl;

    // check find and update
    std::cout << F.value_at(5) << std::endl;
    F.set_value(5, 7);

    // check erase
    F.erase(2);
    F.erase(3);

    // actual output
    for (const auto &p : F) {
        std::cout << p.arg() << " -> " << p.value() << std::endl;
    }

    return 0;
}
