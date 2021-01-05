#include "gtest/gtest.h"
#include "function_maxima.h"
#include <vector>

// EXAMPLE TEST CLASSES.

class Secret {
public:
    int get() const {
        return value;
    }

    bool operator<(const Secret &a) const {
        return value < a.value;
    }

    static Secret create(int v) {
        return Secret(v);
    }

    bool operator==(const Secret &a) const {
        return value == a.value;
    }

private:
    Secret(int v) : value(v) {
    }

    int value;
};

template<typename A, typename V>
struct same {
    bool operator()(const typename FunctionMaxima<A, V>::point_type &p,
                    const std::pair<A, V> &q) {
        return !(p.arg() < q.first) && !(q.first < p.arg()) &&
               !(p.value() < q.second) && !(q.second < p.value());
    }
};

template<typename A, typename V>
bool fun_equal(const FunctionMaxima<A, V> &F,
               const std::initializer_list<std::pair<A, V>> &L) {
    return F.size() == L.size() &&
           std::equal(F.begin(), F.end(), L.begin(), same<A, V>());
}

template<typename A, typename V>
bool fun_mx_equal(const FunctionMaxima<A, V> &F,
                  const std::initializer_list<std::pair<A, V>> &L) {
    return static_cast<typename FunctionMaxima<A, V>::size_type>(std::distance(F.mx_begin(), F.mx_end())) == L.size() &&
           std::equal(F.mx_begin(), F.mx_end(), L.begin(), same<A, V>());
}

TEST(example, example
) {
    FunctionMaxima<int, int> fun;
    fun.set_value(0, 1);
    ASSERT_TRUE(fun_equal(fun, {{0, 1}})
    );
    ASSERT_TRUE(fun_mx_equal(fun, {{0, 1}})
    );

    fun.set_value(0, 0);
    ASSERT_TRUE(fun_equal(fun, {{0, 0}})
    );
    ASSERT_TRUE(fun_mx_equal(fun, {{0, 0}})
    );

    fun.set_value(1, 0);
    fun.set_value(2, 0);
    ASSERT_TRUE(fun_equal(fun, {{0, 0},
                                {1, 0},
                                {2, 0}})
    );
    ASSERT_TRUE(fun_mx_equal(fun, {{0, 0},
                                   {1, 0},
                                   {2, 0}})
    );

    fun.set_value(1, 1);
    ASSERT_TRUE(fun_mx_equal(fun, {{1, 1}})
    );

    fun.set_value(2, 2);
    ASSERT_TRUE(fun_mx_equal(fun, {{2, 2}})
    );
    fun.set_value(0, 2);
    fun.set_value(1, 3);
    ASSERT_TRUE(fun_mx_equal(fun, {{1, 3}})
    );

    try {
// Nie ma wartości na tym argumencie
        auto c = fun.value_at(4);
        (void)
                c;
        ASSERT_TRUE(false);
    } catch (
            InvalidArg &e
    ) {
        std::cout << e.

                what()

                  <<
                  std::endl;
    }

    fun.erase(1);
    ASSERT_TRUE(fun
                        .find(1) == fun.

            end()

    );
    ASSERT_TRUE(fun_mx_equal(fun, {{0, 2},
                                   {2, 2}})
    );

    fun.set_value(-2, 0);
    fun.set_value(-1, -1);
    ASSERT_TRUE(fun_mx_equal(fun, {{0,  2},
                                   {2,  2},
                                   {-2, 0}})
    );

    std::vector<FunctionMaxima<Secret, Secret>::point_type> v;
    {
        FunctionMaxima<Secret, Secret> temp;
        temp.
                set_value(Secret::create(1), Secret::create(10)
        );
        temp.
                set_value(Secret::create(2), Secret::create(20)
        );
        v.
                push_back(*temp
                .

                        begin()

        );
        v.
                push_back(*temp
                .

                        mx_begin()

        );
    }
    ASSERT_TRUE(v[0]
                        .

                                arg()

                        .

                                get()

                == 1);
    ASSERT_TRUE(v[0]
                        .

                                value()

                        .

                                get()

                == 10);
    ASSERT_TRUE(v[1]
                        .

                                arg()

                        .

                                get()

                == 2);
    ASSERT_TRUE(v[1]
                        .

                                value()

                        .

                                get()

                == 20);

// To powinno działać szybko.
    FunctionMaxima<int, int> big;
    using size_type = decltype(big)::size_type;
    const size_type N = 100000;
    for (
            size_type i = 1;
            i <=
            N;
            ++i) {
        big.
                set_value(i, i
        );
    }
    size_type counter = 0;
    for (
            size_type i = 1;
            i <=
            N;
            ++i) {
        big.
                set_value(i, big
                                     .
                                             value_at(i)
                             + 1);
        for (
                auto it = big.mx_begin();
                it != big.

                        mx_end();

                ++it) {
            ++
                    counter;
        }
    }
    ASSERT_TRUE(counter
                == 2 * N - 1);
    big = fun;
}

// CUSTOM TESTS
const int SPECIAL_THROW_VALUE = 42;

class ThrowsOnCompare {
public:
    int get() const {
        return value;
    }

    bool operator<(const ThrowsOnCompare &a) const {
        if (a.value == SPECIAL_THROW_VALUE) {
            throw std::string("BOOM");
        }
        return value < a.value;
    }

    static ThrowsOnCompare create(int v) {
        return ThrowsOnCompare(v);
    }

private:
    explicit ThrowsOnCompare(int v) : value(v) {
    }

    int value;
};

TEST(throwOnCompare, sizeNotChangingAfterChangesValue
) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.
            set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)
    );
    temp.
            set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(20)
    );
    ASSERT_EQ(temp
                      .

                              size(),

              2);
    temp.
            set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(15)
    );
    ASSERT_EQ(temp
                      .

                              size(),

              2);
}

TEST(throwOnCompare, valueChangesProperly
) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.
            set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)
    );
    temp.
            set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(20)
    );
    temp.
            set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(15)
    );
    ASSERT_EQ(temp
                      .

                              size(),

              2);
    auto c = temp.value_at(ThrowsOnCompare::create(2));
    ASSERT_EQ(c
                      .

                              get(),

              15);
}

TEST(throwOnCompare, exceptionHandlingProperly
) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.
            set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)
    );
    EXPECT_THROW(
            {
                temp.
                        set_value(ThrowsOnCompare::create(SPECIAL_THROW_VALUE), ThrowsOnCompare::create(20)
                );
            },
            std::string);
// Wyjątek się rzucił, nie powinno być dodanej liczby.
    ASSERT_EQ(temp
                      .

                              size(),

              1);
}

TEST(throwOnCompare, exceptionHandlingProperlySwapped
) {
    FunctionMaxima<ThrowsOnCompare, ThrowsOnCompare> temp;
    temp.
            set_value(ThrowsOnCompare::create(1), ThrowsOnCompare::create(10)
    );
    EXPECT_THROW(
            {
                temp.
                        set_value(ThrowsOnCompare::create(2), ThrowsOnCompare::create(SPECIAL_THROW_VALUE)
                );
            },
            std::string);
// Wyjątek się rzucił, nie powinno być dodanej liczby.
    ASSERT_EQ(temp
                      .

                              size(),

              1);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
