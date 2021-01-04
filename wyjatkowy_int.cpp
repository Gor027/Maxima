//
// Created by kamil on 04.01.2021.
//
#include <bits/stdc++.h>
#include "function_maxima.h"
using namespace std;


class PorExc : public std::exception {
public:
    const char *what() const noexcept override {
        return "invalid argument value";
    }
};

class WINT {
public:
    WINT(int a) : i(a) {counter++;};

    WINT(const WINT& w) : i(w.i) {counter++;};
    ~WINT() {counter--;}

    const WINT& operator=(const WINT& w) {
        i = w.i;
        return *this;
    }

    bool operator<(const WINT& w) const{
        if (rand()%20 == 1)
            throw PorExc();
        return i < w.i;
    }

    static int get_count() {return counter;}

private:
    int i;
    static int counter;
};

int WINT::counter = 0;

int main(){

    srand(time(0));

    FunctionMaxima<WINT, WINT> fun;

    for(int i = 0; i < 100000; i++) {
        try {
            fun.set_value(WINT(i), WINT(i));
        } catch (exception &e) {
           // cout << "w";
        }
    }

    cout << "JESTEM\n";
    FunctionMaxima<WINT, WINT> fun2(fun);
    FunctionMaxima<WINT, WINT> fun3;
    fun3 = fun2;
    
    assert(fun.size() == fun2.size() && fun3.size() == fun2.size() && fun.size() == WINT::get_count() / 2);

    for(int i = 0; i < 100000; i++) {
        try {
            fun2.erase(WINT(i));
        } catch (exception &e) {
         //   cout << "w";
        }
    }

    assert(fun.size() > fun2.size());
    assert(fun.size() == WINT::get_count() / 2);


    cout << "PRZESZŁO\n";


    return 0;
}