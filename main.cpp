#include "function_maxima.h"

#include "iostream"
#include <set>

class Fun {
    int x;
    int y;

public:

    Fun(int x, int y) : x(x), y(y) {}

    inline bool operator<(const Fun &rhs) const {
        return this->x < rhs.x;
    }

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }
};


int main() {
//    std::set<Fun> mySet;
//
//    mySet.insert(Fun(5, 1));
//    mySet.insert(Fun(6, 2));
//    mySet.insert(Fun(7, 3));
//    mySet.insert(Fun(8, 4));
//    mySet.insert(Fun(9, 5));
//
//    auto it = mySet.find(Fun(8, 0));
//
//    std::cout << "Element's key is: " << it->getX() << " and value is: " << it->getY() << std::endl;

    FunctionMaxima<int, int> F;
    F.set_value(5, 1);
    F.set_value(5, 2);
    F.set_value(5, 3);
    F.set_value(5, 4);
    F.set_value(5, 5);
    for (const auto &p : F) {
        std::cout << p.arg() << " -> " << p.value() << std::endl;
    }

    return 0;
}
