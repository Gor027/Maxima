#include "../../function_maxima.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <random>
#include <initializer_list>

// Klasy BMA i BMV maja prywanty konstrutkor domyslny oraz publicnzy kopiujacy.
// Pozostale konstruktory i operatory= sa usuniete/nie wygenerowane.
class BMA {
    friend bool operator<(const BMA &x, const BMA &y);

    friend BMA getBMA(int x);

public:
    BMA(const BMA &other) = default;

    BMA &operator=(const BMA &other) = delete;

private:
    explicit BMA(int x) : xd(x) {}

    int xd;
};

class BMV {
    friend bool operator<(const BMV &x, const BMV &y);

    friend BMV getBMV(int x);

public:
    BMV(const BMV &other) = default;

    BMV &operator=(const BMV &other) = delete;

private:
    explicit BMV(int x) : xd(x) {}

    int xd;
};

bool operator<(const BMA &x, const BMA &y) {
    return x.xd < y.xd;
}

bool operator<(const BMV &x, const BMV &y) {
    return x.xd < y.xd;
}

BMA getBMA(int x) {
    return BMA(x);
}

BMV getBMV(int x) {
    return BMV(x);
}

class MaximaIntCmp {
public:
    bool operator()(const std::pair<int, int> &a, const std::pair<int, int> &b) const {
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    }
};

using IntMaximaSet = std::set<std::pair<int, int>, MaximaIntCmp>;
using IntMaximaSetIterator = IntMaximaSet::iterator;

IntMaximaSet findMaximas(const std::set<std::pair<int, int>> &points) {
    IntMaximaSet ans;

    for (auto it = points.begin(); it != points.end(); it++) {
        bool leftSmaller = (it == points.begin() || std::prev(it)->second <= it->second);
        bool rightSmaller = (std::next(it) == points.end() || std::next(it)->second <= it->second);

        if (leftSmaller && rightSmaller) {
            ans.insert(*it);
        }
    }

    return ans;
}

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

private:
    Secret(int v) : value(v) {
    }

    int value;
};

// Ten wyjatek nie dziedziczy z std::exception.
struct SomeException {
    explicit SomeException(std::string v) : what(std::move(v)) {}

    std::string what;
};

std::mt19937 twister(std::random_device{}());

int cmpCount = 0;
int jumpLength = 0;
bool throwOnCopyConstruction = false;

int throwAbove = 0;
int cmpLocalCount = 0;
bool throwOnDemand = false;

struct JumpThrower {
    using typ = std::mt19937::result_type;

    JumpThrower() : id(twister()) {}

    explicit JumpThrower(typ _id) : id(_id) {}

    JumpThrower(const JumpThrower &other) : id(other.id) {
        if (throwOnCopyConstruction) {
            throw SomeException("JumpThrower on Copy");
        }
    }

    bool operator<(const JumpThrower &other) const {
        if (++cmpCount % jumpLength == 0) {
            throw SomeException("JumpThrower");
        }

        return id < other.id;
    }

    std::mt19937::result_type id;
};

struct AboveThrower {
    using typ = std::mt19937::result_type;

    explicit AboveThrower(typ _id) : id(_id) {}

    AboveThrower(const AboveThrower &other) : id(other.id) {
        if (throwOnCopyConstruction) {
            throw SomeException("AboveThrower on Copy");
        }
    }

    bool operator<(const AboveThrower &other) const {
        if (++cmpLocalCount > throwAbove) {
            throw SomeException("AboveThrower");
        }

        return id < other.id;
    }

    std::mt19937::result_type id;
};

template<typename A, typename V>
struct same {
    bool operator()(const typename FunctionMaxima<A, V>::point_type &p,
                    const std::pair<A, V> &q) {
        return !(p.arg() < q.first) && !(q.first < p.arg()) &&
               !(p.value() < q.second) && !(q.second < p.value());
    }
};

template<>
struct same<JumpThrower, JumpThrower> {
    bool operator()(const typename FunctionMaxima<JumpThrower, JumpThrower>::point_type &p,
                    const std::pair<JumpThrower, JumpThrower> &q) {
        return !(p.arg().id < q.first.id) && !(q.first.id < p.arg().id) &&
               !(p.value().id < q.second.id) && !(q.second.id < p.value().id);
    }
};

template<>
struct same<AboveThrower, AboveThrower> {
    bool operator()(const typename FunctionMaxima<AboveThrower, AboveThrower>::point_type &p,
                    const std::pair<AboveThrower, AboveThrower> &q) {
        return !(p.arg().id < q.first.id) && !(q.first.id < p.arg().id) &&
               !(p.value().id < q.second.id) && !(q.second.id < p.value().id);
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

template<typename A, typename V>
bool fun_equal(const FunctionMaxima<A, V> &F,
               const std::vector<std::pair<A, V>> &L) {
    return F.size() == L.size() &&
           std::equal(F.begin(), F.end(), L.begin(), same<A, V>());
}

template<typename A, typename V>
bool fun_mx_equal(const FunctionMaxima<A, V> &F,
                  const std::vector<std::pair<A, V>> &L) {
    return static_cast<typename FunctionMaxima<A, V>::size_type>(std::distance(F.mx_begin(), F.mx_end())) == L.size() &&
           std::equal(F.mx_begin(), F.mx_end(), L.begin(), same<A, V>());
}

template<typename A>
bool equivalentArguments(const A &a, const A &b) {
    return !std::less<A>()(a, b) && !std::less<A>()(b, a);
}

template<typename V>
bool equivalentValues(const V &a, const V &b) {
    return !std::less<V>()(a, b) && !std::less<V>()(b, a);
}

template<typename A, typename V>
auto backupFunction(const FunctionMaxima<A, V> &fun) {
    std::vector<std::pair<A, V>> backup;

    for (const auto &p : fun) {
        backup.emplace_back(p.arg(), p.value());
    }

    return backup;
}

template<typename A, typename V>
auto backupMaxima(const FunctionMaxima<A, V> &fun) {
    std::vector<std::pair<A, V>> backup;

    for (auto p = fun.mx_begin(); p != fun.mx_end(); ++p) {
        backup.emplace_back(p->arg(), p->value());
    }

    return backup;
}

#if TEST_NUM == 701

int allocCount = 0;
int throwOnAlloc = 0;

void *operator new(size_t size) {
    void *mem = malloc(size);

    if (mem == nullptr) {
        throw std::bad_alloc{};
    }

    if (++allocCount == throwOnAlloc) {
        free(mem);
        throw std::bad_alloc{};
    }

    return mem;
}

void operator delete(void *mem) noexcept {
    free(mem);
}

#endif


int main() {

    // Test przykladowy.
#if TEST_NUM == 101
    FunctionMaxima<int, int> fun;
    fun.set_value(0, 1);
    assert(fun_equal(fun, { {0, 1} }));
    assert(fun_mx_equal(fun, { {0, 1} }));

    fun.set_value(0, 0);
    assert(fun_equal(fun, { {0, 0} }));
    assert(fun_mx_equal(fun, { {0, 0} }));

    fun.set_value(1, 0);
    fun.set_value(2, 0);
    assert(fun_equal(fun, { {0, 0}, {1, 0}, {2, 0} }));
    assert(fun_mx_equal(fun, { {0, 0}, {1, 0}, {2, 0} }));

    fun.set_value(1, 1);
    assert(fun_mx_equal(fun, { {1, 1} }));

    fun.set_value(2, 2);
    assert(fun_mx_equal(fun, { {2, 2} }));
    fun.set_value(0, 2);
    fun.set_value(1, 3);
    assert(fun_mx_equal(fun, { {1, 3} }));

    try {
        std::cout << fun.value_at(4) << std::endl;
        assert(false);
    }
    catch (InvalidArg& e) {
        std::cout << e.what() << std::endl;
    }

    fun.erase(1);
    assert(fun.find(1) == fun.end());
    assert(fun_mx_equal(fun, { {0, 2}, {2, 2} }));

    fun.set_value(-2, 0);
    fun.set_value(-1, -1);
    assert(fun_mx_equal(fun, { {0, 2}, {2, 2}, {-2, 0} }));

    std::vector<FunctionMaxima<Secret, Secret>::point_type> v;
    {
        FunctionMaxima<Secret, Secret> temp;
        temp.set_value(Secret::create(1), Secret::create(10));
        temp.set_value(Secret::create(2), Secret::create(20));
        v.push_back(*temp.begin());
        v.push_back(*temp.mx_begin());
    }
    assert(v[0].arg().get() == 1);
    assert(v[0].value().get() == 10);
    assert(v[1].arg().get() == 2);
    assert(v[1].value().get() == 20);

    // To powinno działać szybko.
    FunctionMaxima<int, int> big;
    using size_type = decltype(big)::size_type;
    const size_type N = 100000;
    for (size_type i = 1; i <= N; ++i) {
        big.set_value(i, i);
    }
    size_type counter = 0;
    for (size_type i = 1; i <= N; ++i) {
        big.set_value(i, big.value_at(i) + 1);
        for (auto it = big.mx_begin(); it != big.mx_end(); ++it) {
            ++counter;
        }
    }
    assert(counter == 2 * N - 1);
    big = fun;

#endif

    // Testy 102 -  reczne, male, na odbugowanie drobnych edge caseow.
#if TEST_NUM == 102
    FunctionMaxima<BMA, BMV> fun;

    assert(fun.size() == 0);

    assert(fun.begin() == fun.end());
    assert(fun.mx_begin() == fun.mx_end());

    decltype(fun)::iterator it = fun.begin();
    decltype(fun)::mx_iterator mxIt = fun.mx_begin();

    decltype(fun)::size_type sz = fun.size();

#endif

#if TEST_NUM == 103
    FunctionMaxima<BMA, BMV> fun;

    fun.set_value(getBMA(3), getBMV(69));

    assert(fun.size() == 1);

    assert(equivalentArguments(fun.find(getBMA(3))->arg(), getBMA(3)));
    assert(equivalentValues(fun.find(getBMA(3))->value(), getBMV(69)));

    for (decltype(fun)::point_type p : fun) {
        assert(equivalentArguments(p.arg(), getBMA(3)));
        assert(equivalentValues(p.value(), getBMV(69)));
    }

    assert(equivalentArguments(fun.mx_begin()->arg(), getBMA(3)));
    assert(equivalentValues(fun.mx_begin()->value(), getBMV(69)));

    decltype(fun)::point_type p1 = *(fun.mx_begin());
    decltype(fun)::point_type p2(p1);
    decltype(fun)::point_type p3 = p2;

#endif

#if TEST_NUM == 104
    FunctionMaxima<BMA, BMV> fun;

    fun.set_value(getBMA(3), getBMV(69));

    fun.erase(getBMA(3));

    fun.set_value(getBMA(3), getBMV(69));
    fun.set_value(getBMA(3), getBMV(69));
    fun.set_value(getBMA(3), getBMV(69));

    assert(fun_equal(fun, { {getBMA(3), getBMV(69)} }));
    assert(fun_mx_equal(fun, { {getBMA(3), getBMV(69)} }));

    fun.set_value(getBMA(3), getBMV(21));
    fun.set_value(getBMA(3), getBMV(37));

    assert(fun_equal(fun, { {getBMA(3), getBMV(37)} }));
    assert(fun_mx_equal(fun, { {getBMA(3), getBMV(37)} }));

    fun.erase(getBMA(3));

    fun.erase(getBMA(4));

    fun.erase(getBMA(5));

    fun.set_value(getBMA(4), getBMV(6));

    assert(fun.size() == 1);

    assert(equivalentValues(fun.find(getBMA(4))->value(), getBMV(6)));

#endif

#if TEST_NUM == 105
    FunctionMaxima<int, int> fun;

    fun.set_value(1, 2137);
    fun.set_value(1, 2137);

    assert(fun_mx_equal(fun, { {1, 2137} }));

    fun.set_value(2, 2137);
    fun.set_value(2, 2137);

    assert(fun_mx_equal(fun, { {1, 2137}, {2, 2137} }));

    fun.set_value(3, 2137);
    fun.set_value(3, 2137);

    assert(fun_equal(fun, { {1, 2137}, {2, 2137}, {3, 2137} }));
    assert(fun_mx_equal(fun, { {1, 2137}, {2, 2137}, {3, 2137} }));

    fun.set_value(2, 69);
    fun.set_value(2, 69);

    assert(fun_equal(fun, { {1, 2137}, {2, 69}, {3, 2137} }));
    assert(fun_mx_equal(fun, { {1, 2137}, {3, 2137}  }));

    fun.set_value(2, 5000);
    fun.set_value(2, 5000);

    assert(fun_equal(fun, { {1, 2137}, {2, 5000}, {3, 2137} }));
    assert(fun_mx_equal(fun, { {2, 5000} }));

    assert(fun.size() == 3);

    assert(fun.value_at(1) == 2137);
    assert(fun.value_at(2) == 5000);
    assert(fun.value_at(3) == 2137);

    assert(fun.find(1)->arg() == 1);
    assert(fun.find(1)->value() == 2137);

    assert(fun.find(2)->arg() == 2);
    assert(fun.find(2)->value() == 5000);

    assert(fun.find(3)->arg() == 3);
    assert(fun.find(3)->value() == 2137);

    assert(fun.find(4) == fun.end());

#endif

#if TEST_NUM == 106
    FunctionMaxima<int, int> fun;

    fun.set_value(1, 21);
    fun.set_value(2, 37);

    assert(fun_equal(fun, { {1, 21}, {2, 37} }));
    assert(fun_mx_equal(fun, { {2, 37} }));

    fun.set_value(1, 37);
    fun.set_value(2, 21);

    assert(fun_equal(fun, { {1, 37}, {2, 21} }));
    assert(fun_mx_equal(fun, { {1, 37} }));

    fun.set_value(2, 37);

    assert(fun_equal(fun, { {1, 37}, {2, 37} }));
    assert(fun_mx_equal(fun, { {1, 37}, {2, 37} }));

    fun.erase(2);

    assert(fun_equal(fun, { {1, 37} }));
    assert(fun_mx_equal(fun, { {1, 37} }));

    fun.set_value(1, 37);

    fun.erase(1);

    fun.set_value(2, 37);

    assert(fun_equal(fun, { {2, 37} }));
    assert(fun_mx_equal(fun, { {2, 37} }));
#endif

    // Sprawdzenie algorytmicznej poprawnosci.
#if TEST_NUM == 201
    constexpr int N = 8;
    constexpr int B = 4;

    int power = 1;

    for (int i = 1; i <= N; i++) {
        power *= B;
    }

    std::vector<int> prevMask(N + 1);

    FunctionMaxima<int, int> fun;

    for (int mask = 0; mask < power; mask++) {
        int aux = mask;

        std::set<std::pair<int, int> > points;

        for (int i = N; i >= 1; i--) {
            int digit = aux % B;
            aux /= B;

            if (digit != prevMask[i]) {
                if (digit == 0) {
                    fun.erase(i);
                }
                else {
                    fun.set_value(i, digit);
                }
            }

            if (digit != 0) {
                points.emplace(i, digit);
            }

            prevMask[i] = digit;
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 202
    std::vector<int> id = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::vector<int> perm = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};

    std::random_device rd;
    std::mt19937 g(rd());

    do {
        FunctionMaxima<int, int> fun;

        std::shuffle(id.begin(), id.end(), g);

        std::set<std::pair<int, int> > points;

        for (int i = 0; i < 12; i++) {
            points.emplace(id[i], perm[i]);

            fun.set_value(id[i], perm[i]);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));

    } while (std::next_permutation(perm.begin(), perm.end()));
#endif

#if TEST_NUM == 203
    std::vector<int> id = { 1, 2, 3, 4, 5, 6, 7, 8};
    std::vector<int> perm = { 1, 1, 2, 2, 3, 3, 4, 4};

    std::random_device rd;
    std::mt19937 g(rd());

    do {
        FunctionMaxima<int, int> fun;

        std::shuffle(id.begin(), id.end(), g);

        std::set<std::pair<int, int> > points;

        for (int i = 0; i < 8; i++) {
            points.emplace(id[i], perm[i]);

            fun.set_value(id[i], perm[i]);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));

    } while (std::next_permutation(perm.begin(), perm.end()));

#endif

#if TEST_NUM == 204
    constexpr int N = 8;
    constexpr int B = 5;

    int power = 1;

    for (int i = 1; i <= N; i++) {
        power *= B;
    }

    std::vector<int> prevMask(N + 1);

    FunctionMaxima<int, int> fun;

    for (int mask = 0; mask < power; mask++) {
        int aux = mask;

        std::set<std::pair<int, int> > points;

        for (int i = N; i >= 1; i--) {
            int digit = aux % B;
            aux /= B;

            if (digit != prevMask[i]) {
                if (digit == 0) {
                    fun.erase(i);
                }
                else {
                    fun.set_value(i, digit);
                }
            }

            if (digit != 0) {
                points.emplace(i, digit);
            }

            prevMask[i] = digit;
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 205
    constexpr int OPC = 20000;
    constexpr int N = 100;
    constexpr int M = 7;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 5);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({a, -1});
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        } else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({a, -1});
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 206
    constexpr int OPC = 20000;
    constexpr int N = 20;
    constexpr int M = 7;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 5);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        }
        else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 207
    constexpr int OPC = 100000;
    constexpr int N = 20;
    constexpr int M = 5;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 5);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        }
        else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 208
    constexpr int OPC = 100000;
    constexpr int N = 20;
    constexpr int M = 5;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 3);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        }
        else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 209
    constexpr int OPC = 100000;
    constexpr int N = 20;
    constexpr int M = 5;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 2);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        }
        else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 210
    constexpr int OPC = 100000;
    constexpr int N = 7;
    constexpr int M = 5;

    std::default_random_engine eO(85768369);
    std::uniform_int_distribution<> op(1, 3);

    std::default_random_engine eA(38877271);
    std::uniform_int_distribution<> arg(1, N);

    std::default_random_engine eV(184271);
    std::uniform_int_distribution<> value(1, M);

    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int> > points;

    for (int i = 1; i <= OPC; i++) {
        if (op(eO) == 1) {
            int a = arg(eA);
            fun.erase(a);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }
        }
        else {
            int a = arg(eA);
            int v = value(eV);
            fun.set_value(a, v);

            auto it = points.lower_bound({ a, -1 });
            if (it != points.end() && it->first == a) {
                points.erase(it);
            }

            points.emplace(a, v);
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }
#endif

#if TEST_NUM == 211
    constexpr int N = 8;
    constexpr int B = 4;

    int power = 1;

    for (int i = 1; i <= N; i++) {
        power *= B;
    }

    std::vector<int> masks;

    masks.reserve(power);

    for (int mask = 0; mask < power; mask++) {
        masks.push_back(mask);
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(masks.begin(), masks.end(), g);

    FunctionMaxima<int, int> fun;

    std::vector<int> prevMask(N + 1);

    for (auto mask : masks) {
        int aux = mask;

        std::set<std::pair<int, int> > points;

        for (int i = N; i >= 1; i--) {
            int digit = aux % B;
            aux /= B;

            if (digit != prevMask[i]) {
                if (digit == 0) {
                    fun.erase(i);
                } else {
                    fun.set_value(i, digit);
                }
            }

            if (digit != 0) {
                points.emplace(i, digit);
            }

            prevMask[i] = digit;
        }

        IntMaximaSet mxPoints = findMaximas(points);

        assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
        assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));
    }

#endif

    // Kopiowanie i przypisywanie.
#if TEST_NUM == 301
    FunctionMaxima<BMA, BMV> fun;

    fun.set_value(getBMA(2), getBMV(21));
    fun.set_value(getBMA(1), getBMV(37));
    fun.set_value(getBMA(3), getBMV(37));

    FunctionMaxima<BMA, BMV> fun2(fun);
    FunctionMaxima<BMA, BMV> fun3(fun);

    fun2.erase(getBMA(2));
    fun3.erase(getBMA(3));

    assert(fun_equal(fun, {{getBMA(1), getBMV(37)},
                           {getBMA(2), getBMV(21)},
                           {getBMA(3), getBMV(37)}}));
    assert(fun_mx_equal(fun, {{getBMA(1), getBMV(37)},
                              {getBMA(3), getBMV(37)}}));

    assert(fun_equal(fun2, {{getBMA(1), getBMV(37)},
                            {getBMA(3), getBMV(37)}}));
    assert(fun_mx_equal(fun2, {{getBMA(1), getBMV(37)},
                               {getBMA(3), getBMV(37)}}));

    assert(fun_equal(fun3, {{getBMA(1), getBMV(37)},
                            {getBMA(2), getBMV(21)}}));
    assert(fun_mx_equal(fun3, {{getBMA(1), getBMV(37)}}));

    FunctionMaxima<BMA, BMV> fun4 = fun2;

    fun2.set_value(getBMA(1), getBMV(37));

    fun2.erase(getBMA(1));

    fun2.set_value(getBMA(1), getBMV(46));
    fun2.set_value(getBMA(4), getBMV(100));
    fun2.set_value(getBMA(2), getBMV(100));

    fun.set_value(getBMA(5), getBMV(100));

    fun4.erase(getBMA(3));
    fun4.set_value(getBMA(6), getBMV(100));

    assert(fun_equal(fun, {{getBMA(1), getBMV(37)},
                           {getBMA(2), getBMV(21)},
                           {getBMA(3), getBMV(37)},
                           {getBMA(5), getBMV(100)}}));
    assert(fun_mx_equal(fun, {{getBMA(5), getBMV(100)},
                              {getBMA(1), getBMV(37)}}));

    assert(fun_equal(fun2, {{getBMA(1), getBMV(46)},
                            {getBMA(2), getBMV(100)},
                            {getBMA(3), getBMV(37)},
                            {getBMA(4), getBMV(100)}}));
    assert(fun_mx_equal(fun2, {{getBMA(2), getBMV(100)},
                               {getBMA(4), getBMV(100)}}));

    assert(fun_equal(fun4, {{getBMA(1), getBMV(37)},
                            {getBMA(6), getBMV(100)}}));
    assert(fun_mx_equal(fun4, {{getBMA(6), getBMV(100)}}));

    fun = fun;
    fun2 = fun2;
    fun4 = fun4;

    assert(fun_equal(fun, {{getBMA(1), getBMV(37)},
                           {getBMA(2), getBMV(21)},
                           {getBMA(3), getBMV(37)},
                           {getBMA(5), getBMV(100)}}));
    assert(fun_mx_equal(fun, {{getBMA(5), getBMV(100)},
                              {getBMA(1), getBMV(37)}}));

    assert(fun_equal(fun2, {{getBMA(1), getBMV(46)},
                            {getBMA(2), getBMV(100)},
                            {getBMA(3), getBMV(37)},
                            {getBMA(4), getBMV(100)}}));
    assert(fun_mx_equal(fun2, {{getBMA(2), getBMV(100)},
                               {getBMA(4), getBMV(100)}}));

    assert(fun_equal(fun4, {{getBMA(1), getBMV(37)},
                            {getBMA(6), getBMV(100)}}));
    assert(fun_mx_equal(fun4, {{getBMA(6), getBMV(100)}}));
#endif

    // Grupa 4 -  Exception safety.

    // Mieszany rzucacz odstepowy :).
#if TEST_NUM == 401
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 5);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 1000; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                fun.set_value(JumpThrower{u(e)}, JumpThrower{u(e)});
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }

#endif

#if TEST_NUM == 402
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 100);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 100; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                fun.set_value(JumpThrower{u(e)}, JumpThrower{u(e)});
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }

#endif

#if TEST_NUM == 403
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 5);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 100; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                if (u(e) == 1) {
                    fun.erase(JumpThrower{u(e)});
                } else {
                    fun.set_value(JumpThrower{u(e)}, JumpThrower{u(e)});
                }
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }
#endif

#if TEST_NUM == 404
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 20);

    std::default_random_engine e2(62772949);
    std::uniform_int_distribution<JumpThrower::typ> u2(1, 4);

    std::default_random_engine e3(119611);
    std::uniform_int_distribution<JumpThrower::typ> u3(1, 5);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 1000; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                if (u2(e2) == 1) {
                    fun.erase(JumpThrower{u(e)});
                } else {
                    fun.set_value(JumpThrower{u(e)}, JumpThrower{u3(e3)});
                }
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }
#endif

#if TEST_NUM == 405
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 7);

    std::default_random_engine e2(62772949);
    std::uniform_int_distribution<JumpThrower::typ> u2(1, 3);

    std::default_random_engine e3(119611);
    std::uniform_int_distribution<JumpThrower::typ> u3(1, 5);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 1000; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                if (u2(e2) == 1) {
                    fun.erase(JumpThrower{u(e)});
                } else {
                    fun.set_value(JumpThrower{u(e)}, JumpThrower{u3(e3)});
                }
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }
#endif

#if TEST_NUM == 406
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<JumpThrower::typ> u(1, 7);

    std::default_random_engine e2(62772949);
    std::uniform_int_distribution<JumpThrower::typ> u2(1, 3);

    std::default_random_engine e3(119611);
    std::uniform_int_distribution<JumpThrower::typ> u3(1, 5);

    for (int jump = 1; jump <= 200; jump++) {
        FunctionMaxima<JumpThrower, JumpThrower> fun;

        jumpLength = jump;
        cmpCount = 0;
        throwOnCopyConstruction = false;

        for (int i = 1; i <= 1000; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            try {
                if (u2(e2) == 1) {
                    fun.erase(JumpThrower{u(e)});
                } else {
                    fun.set_value(JumpThrower{u(e)}, JumpThrower{u3(e3)});
                }
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }
#endif

#if TEST_NUM == 407
    std::default_random_engine e(38877271);
    std::uniform_int_distribution<AboveThrower::typ> u(1, 5);

    std::default_random_engine e2(62772949);
    std::uniform_int_distribution<AboveThrower::typ> u2(1, 3);

    std::default_random_engine e3(119611);
    std::uniform_int_distribution<AboveThrower::typ> u3(1, 5);

    for (int above = 1; above <= 200; above++) {
        FunctionMaxima<AboveThrower, AboveThrower> fun;

        for (int i = 1; i <= 20000; i++) {
            auto funBackup = backupFunction(fun);
            auto maximaBackup = backupMaxima(fun);

            if (i % 20 == 0) throwAbove = above;
            else throwAbove = 300000;

            cmpLocalCount = 0;

            try {
                if (u2(e2) == 1) {
                    fun.erase(AboveThrower{u(e)});
                } else {
                    fun.set_value(AboveThrower{u(e)}, AboveThrower{u3(e3)});
                }
            }
            catch (const SomeException &) {
                assert(fun_equal(fun, funBackup));
                assert(fun_mx_equal(fun, maximaBackup));
            }
        }
    }

#endif

    // Testy, ktore nie powinny sie kompilowac.
#if TEST_NUM == 501
    FunctionMaxima<Secret, Secret>::point_type p;
    FunctionMaxima<int, int>::point_type p2(1, 2);
#endif

    // Testy na Copy-On-Write.
#if TEST_NUM == 601
    FunctionMaxima<int, int> fun;

    std::set<std::pair<int, int>> points;

    for (int i = 1; i <= 100'000; i++) {
        fun.set_value(i, i);
        points.emplace(i, i);
    }

    FunctionMaxima<int, int> funArray[500'000];

    for (int i = 0; i < 500'000; i++) {
        funArray[i] = fun;
    }

    funArray[1].set_value(6, 1);
    funArray[2].set_value(7, 2);

    fun.set_value(8, 3);

    std::set<std::pair<int, int>> points1 = points;

    points1.erase({6, 6});
    points1.emplace(6, 1);

    std::set<std::pair<int, int>> points2 = points;

    points2.erase({7, 7});
    points2.emplace(7, 2);

    points.erase({8, 8});
    points.emplace(8, 3);

    IntMaximaSet mxPoints = findMaximas(points);
    IntMaximaSet mxPoints1 = findMaximas(points1);
    IntMaximaSet mxPoints2 = findMaximas(points2);

    assert(fun_equal(fun, std::vector<std::pair<int, int>>(points.begin(), points.end())));
    assert(fun_mx_equal(fun, std::vector<std::pair<int, int>>(mxPoints.begin(), mxPoints.end())));

    assert(fun_equal(funArray[1], std::vector<std::pair<int, int>>(points1.begin(), points1.end())));
    assert(fun_mx_equal(funArray[1], std::vector<std::pair<int, int>>(mxPoints1.begin(), mxPoints1.end())));

    assert(fun_equal(funArray[2], std::vector<std::pair<int, int>>(points2.begin(), points2.end())));
    assert(fun_mx_equal(funArray[2], std::vector<std::pair<int, int>>(mxPoints2.begin(), mxPoints2.end())));

#endif

    // Test na silna gwarancje operator=.
#if TEST_NUM == 701
    for (int allocNo = 1; allocNo <= 200; allocNo++) {
        allocCount = 0;
        throwOnAlloc = 0;

        FunctionMaxima<int, int> fun, fun2;

        fun.set_value(245, 1923);
        fun.set_value(6542, 5473);
        fun.set_value(17546, 8456);

        fun2.set_value(21, 54);
        fun2.set_value(69, 91);
        fun2.set_value(37, 42);

        std::vector<std::pair<int, int>> funBackup = backupFunction(fun);
        std::vector<std::pair<int, int>> maximaBackup = backupMaxima(fun);

        std::vector<std::pair<int, int>> funBackup2 = backupFunction(fun2);
        std::vector<std::pair<int, int>> maximaBackup2 = backupMaxima(fun2);

        allocCount = 0;
        throwOnAlloc = allocNo;

        try {
            fun = fun2;

            throwOnAlloc = 0;
            assert(fun_equal(fun, funBackup2));
            assert(fun_mx_equal(fun, maximaBackup2));
        } catch (const std::bad_alloc &) {
            assert(fun_equal(fun, funBackup));
            assert(fun_mx_equal(fun, maximaBackup));
        }
    }

#endif
}
