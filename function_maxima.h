#ifndef MAXIMA_FUNCTION_MAXIMA_H
#define MAXIMA_FUNCTION_MAXIMA_H

#include "iostream"
#include <map>
#include <set>
#include <vector>
#include <memory>
#include "experimental/propagate_const"
#include "iterator"

/*********************************INVALID_ARG*********************************/

class InvalidArg : public std::exception {
public:
    explicit InvalidArg(const char *message) : errorMessage(message) {}

    virtual const char *what() const noexcept {
        return errorMessage;
    }

private:
    const char *errorMessage;
};

/*********************************FUNCTION_MAXIMA*********************************/

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type;

    using size_type = std::size_t;

    using iterator = typename std::set<point_type>::iterator;
    using mx_iterator = typename std::set<point_type>::iterator;

    explicit FunctionMaxima();

    /**
     * Copy constructors
     */
    FunctionMaxima(const FunctionMaxima<A, V> &rhs) : pImpl(std::make_unique<Impl>(*rhs.pImpl)) {
    }

    FunctionMaxima &operator=(const FunctionMaxima &rhs) {
        pImpl = std::make_unique<Impl>(*rhs.pImpl);

        return *this;
    }

    /**
     * Destructor
     */
    ~FunctionMaxima() = default;

    /***********GOES_INTO_IMPL***********/

    V const &value_at(A const &a) const;

    void set_value(A const &a, V const &v);

    void erase(A const &a);

    iterator begin() const;

    iterator end() const;

    iterator find(A const &a) const;

    mx_iterator mx_begin() const;

    mx_iterator mx_end() const;

    size_type size() const;

private:
    class Impl;

    std::unique_ptr<Impl> pImpl;
};

/*********************************POINT_TYPE*********************************/

template<typename A, typename V>
class FunctionMaxima<A, V>::point_type {
public:
    A const &arg() const {
        return *argument.get();
    }

    V const &value() const {
        return *point.get();
    }

    point_type(const point_type &rhs) = default;

    point_type(point_type &&rhs) noexcept = default;

private:
    /**
     * Impl class will have access to the private parametrized constructor of point_type
     */
    friend class FunctionMaxima<A, V>::Impl;

    point_type(A argument, V point) : argument(std::make_shared<A>(argument)), point(std::make_shared<V>(point)) {}

    point_type(A argument) : argument(std::make_shared<A>(argument)), point(nullptr) {}

    std::shared_ptr<A> argument;
    std::shared_ptr<V> point;
};

/*********************************FUNCTION_MAXIMA_IMPL*********************************/

template<typename A, typename V>
class FunctionMaxima<A, V>::Impl {
public:
    Impl() = default;

    V const &value_at(const A &a) const {
        point_type toSearch = {a};

        auto it = pointSet.find(toSearch);

        if (it == pointSet.end()) {
            throw InvalidArg("The argument is out of the domain.");
        }

        return ((point_type) *it).value();
    }

    void set_value(const A &a, const V &v) {
        std::vector<mx_iterator> success;
        std::vector<mx_iterator> rollback;
        std::vector<iterator> surrounding;
        bool insertion = false;
        success.reserve(requiredSpace);
        rollback.reserve(requiredSpace);
        surrounding.reserve(requiredSpace);

        try {
            point_type toInsert = {a, v};

            surrounding.push_back(pointSet.find(toInsert));

            if (surrounding[prevMiddle] == pointSet.end()) {
                surrounding.push_back(pointSet.insert(toInsert));
                insertion = true;
                surrounding.push_back(moveItLeft(surrounding[newMiddle]));
                surrounding.push_back(moveItRight(surrounding[newMiddle]));
            } else {
                surrounding.push_back(surrounding[prevMiddle]);
                surrounding.push_back(moveItLeft(surrounding[prevMiddle]));
                surrounding.push_back(moveItRight(surrounding[prevMiddle]));
                surrounding[newMiddle] = pointSet.insert(toInsert);
                insertion = true;
            }

            surrounding.push_back(moveItLeft(surrounding[left]));
            surrounding.push_back(moveItRight(surrounding[right]));

            updateMaximum(surrounding[leftmost], surrounding[left], surrounding[newMiddle],
                          success, rollback);
            updateMaximum(surrounding[newMiddle], surrounding[right], surrounding[rightmost],
                          success, rollback);
            updateMaximum(surrounding[left], surrounding[newMiddle], surrounding[right],
                          success, rollback);

            if (surrounding[prevMiddle] != pointSet.end()) {
                success.push_back(maximaPointSet.find(*surrounding[prevMiddle]));
            }
        }
        catch (...) {
            for (size_t i = 0; i < rollback.size(); i++) {
                if (rollback[i] != maximaPointSet.end()) {
                    maximaPointSet.erase(rollback[i]);
                }
            }

            if (insertion && surrounding[newMiddle] != pointSet.end()) {
                pointSet.erase(surrounding[newMiddle]);
            }

            throw;
        }

        for (size_t i = 0; i < success.size(); i++) {
            if (success[i] != maximaPointSet.end()) {
                maximaPointSet.erase(success[i]);
            }
        }

        if (surrounding[prevMiddle] != pointSet.end()) {
            pointSet.erase(surrounding[prevMiddle]);
        }
    }

    void erase(const A &a) {
        std::vector<mx_iterator> success;
        std::vector<mx_iterator> rollback;
        std::vector<iterator> surrounding;
        success.reserve(requiredSpace);
        rollback.reserve(requiredSpace);
        surrounding.reserve(requiredSpace);

        try {
            point_type toRemove = {a};
            surrounding.push_back(pointSet.find(toRemove));
            surrounding.push_back(pointSet.find(toRemove));

            if (surrounding[prevMiddle] == pointSet.end()) {
                return;
            }

            surrounding.push_back(moveItLeft(surrounding[prevMiddle]));
            surrounding.push_back(moveItRight(surrounding[prevMiddle]));
            surrounding.push_back(moveItLeft(surrounding[left]));
            surrounding.push_back(moveItRight(surrounding[right]));

            updateMaximum(surrounding[leftmost], surrounding[left], surrounding[right],
                          success, rollback);
            updateMaximum(surrounding[left], surrounding[right], surrounding[rightmost],
                          success, rollback);

            if (surrounding[prevMiddle] != pointSet.end()) {
                success.push_back(maximaPointSet.find(*surrounding[prevMiddle]));
            }
        }
        catch (...) {
            for (size_t i = 0; i < rollback.size(); i++) {
                if (rollback[i] != maximaPointSet.end()) {
                    maximaPointSet.erase(rollback[i]);
                }
            }

            throw;
        }

        for (size_t i = 0; i < success.size(); i++) {
            if (success[i] != maximaPointSet.end()) {
                maximaPointSet.erase(success[i]);
            }
        }

        if (surrounding[prevMiddle] != pointSet.end()) {
            pointSet.erase(surrounding[prevMiddle]);
        }
    }

    FunctionMaxima<A, V>::iterator begin() const {
        return pointSet.begin();
    }

    FunctionMaxima<A, V>::iterator end() const {
        return pointSet.end();
    }

    FunctionMaxima<A, V>::iterator find(A const &a) const {
        point_type toSearch = {a};

        return pointSet.find(toSearch);
    }

    FunctionMaxima<A, V>::mx_iterator mx_begin() const {
        return maximaPointSet.begin();
    }

    FunctionMaxima<A, V>::mx_iterator mx_end() const {
        return maximaPointSet.end();
    }

    size_type size() const {
        return pointSet.size();
    }

private:

    enum {
        prevMiddle,
        newMiddle,
        left,
        right,
        leftmost,
        rightmost,
        requiredSpace = 10
    };

    iterator moveItLeft(iterator it) {
        auto tempIt = it;

        if (it != pointSet.end() && it != pointSet.begin()) {
            tempIt--;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    iterator moveItRight(iterator it) {
        auto tempIt = it;

        if (it != pointSet.end()) {
            tempIt++;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    static bool sameValue(point_type p1, point_type p2) {
        return (!(p1.value() < p2.value()) && !(p2.value() < p1.value()));
    }

    bool shouldBeMaximum(iterator leftIt, iterator it, iterator rightIt) const {
        return (leftIt == pointSet.end() || leftIt->value() < it->value() ||
                sameValue(*leftIt, *it)) &&
               (rightIt == pointSet.end() || rightIt->value() < it->value() ||
                sameValue(*rightIt, *it));
    }

    void updateMaximum(iterator leftIt, iterator it, iterator rightIt,
                       std::vector<iterator> &success, std::vector<iterator> &rollback) {

        if (it == pointSet.end()) {
            return;
        }

        auto maximaIt = maximaPointSet.find(*it);
        bool checkNew = shouldBeMaximum(leftIt, it, rightIt);

        if (maximaIt != maximaPointSet.end() && !checkNew) {
            success.push_back(maximaIt);
        }

        if (maximaIt == maximaPointSet.end() && checkNew) {
            rollback.push_back(maximaPointSet.insert(*it));
        }
    }

    struct pointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.arg() < b.arg();
        }
    };

    struct maximaPointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return (
                    (b.value() < a.value()) ||
                    (sameValue(a, b) &&
                     (a.arg() < b.arg()))
            );
        }
    };

    std::multiset<point_type, pointSetCmp> pointSet;
    std::multiset<point_type, maximaPointSetCmp> maximaPointSet;
};

/**
 * No-parameter constructor for FunctionMaxima
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 */
template<typename A, typename V>
FunctionMaxima<A, V>::FunctionMaxima() : pImpl{std::make_unique<Impl>()} {
}

/**
 * The function will look up for the key in the map.
 * In case there is no such key, Invalid Argument exception is thrown.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - const reference to the key to be searched
 * @return the value of the found key
 */
template<typename A, typename V>
V const &FunctionMaxima<A, V>::value_at(const A &a) const {
    return pImpl->value_at(a);
}

/**
 * The function will update the value of the existing key.
 * If the key does not exist in the map, then it will be inserted
 * with the value assigned to it.
 *
 * Note: The function first erases element by key and the inserts the new one.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - const reference to the key to be updated
 * @param v - const reference to the value to be assigned to a key
 */
template<typename A, typename V>
void FunctionMaxima<A, V>::set_value(const A &a, const V &v) {
    return pImpl->set_value(a, v);
}

/**
 * The function erases the element given by the key.
 * Note that if the element is a point then the pointed-to memory will not be touched.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - const reference to the key of the element to be removed
 */
template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
    return pImpl->erase(a);
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::begin() const {
    return pImpl->begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::end() const {
    return pImpl->end();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::find(const A &a) const {
    return pImpl->find(a);
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_begin() const {
    return pImpl->mx_begin();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_end() const {
    return pImpl->mx_end();
}

template<typename A, typename V>
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const {
    return pImpl->size();
}

#endif //MAXIMA_FUNCTION_MAXIMA_H
