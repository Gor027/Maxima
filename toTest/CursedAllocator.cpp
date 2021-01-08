/**
 * Test for checking whether your program correctly implements operator=(FunctionMaxima const &)
 * Note that set::operator=(set const &) does not have strong guarantees.
 * This test only applies to implementations not using Copy-on-Write or similar techniques.
 *
 * Written by Pawel Pawlowski, released under public domain.
 */

#include "../function_maxima.h"
#include <malloc.h>
#include <cstdlib>
#include <stdexcept>

// https://stackoverflow.com/a/17850402
extern "C" void *__libc_malloc(size_t size);
static thread_local int malloc_break = 0;
void *malloc (size_t size) {
    if (malloc_break > 0 && rand() < malloc_break) {
        return nullptr;
    }
    return __libc_malloc(size);
}
void set_malloc_break(double value) {
    malloc_break = (int) (value * RAND_MAX);
}

int main() {
    // 1. Create a unique-ish but simple function (this shouldn't throw)
    FunctionMaxima<int, int> fun;
    for (int i = 0; i < 10000; i++)
        fun.set_value(-i, 10000 + i - (i % 2 == 0 ? 10 : 0));

    // 2. Stress test assigning with a small chance of malloc failing
    for (int i = 0; i < 10000; i++) {
        constexpr size_t expectedValueCount = 100;
        int expectedValues[expectedValueCount];
        for (int &expectedValue : expectedValues)
            expectedValue = rand();

        FunctionMaxima<int, int> fun2;
        for (int j = 0; j < expectedValueCount; j++) // whatever really
            fun2.set_value(j, expectedValues[j]);

        // NOTE: If you want to ensure that the value checking part of this program works correctly you can comment this section out
        set_malloc_break(0.0001f); // you can try to change this value but this one gave good results at failing my program
        try {
            fun2 = fun; // copy assignment
            set_malloc_break(0.f);
            continue; // this was a success - no need to check
        } catch (std::bad_alloc &) {
            // ignore
        }
        set_malloc_break(0.f);

        // Check the values
        int c = 0;
        for (auto const &p : fun2) {
            if (p.arg() != c || p.value() != expectedValues[c])
                throw std::runtime_error("key/value mismatch");
            if (++c > expectedValueCount)
                throw std::runtime_error("too many entries");
        }
        if (c != expectedValueCount)
            throw std::runtime_error("not enough entries");

        // Check the maximums
        std::set<std::pair<int, int>> maximums;
        for (int j = 0; j < expectedValueCount; j++) {
            if ((j == 0 || expectedValues[j - 1] <= expectedValues[j]) &&
                (j == expectedValueCount - 1 || expectedValues[j + 1] <= expectedValues[j])) {
                maximums.insert({-expectedValues[j], j});
            }
        }

        auto it = fun2.mx_begin();
        for (auto const &m : maximums) {
            if (it == fun2.mx_end())
                throw std::runtime_error("not enough maximums");
            auto j = m.second;
            if (it->arg() != j || it->value() != expectedValues[j])
                throw std::runtime_error("key/value mismatch in maximum");
            ++it;
        }
        if (it != fun2.mx_end())
            throw std::runtime_error("too many maximums");
    }
}