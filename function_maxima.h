//
// Created by gor027 on 15.12.2020.
//

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
     * Copy constructor
     */
    FunctionMaxima(const FunctionMaxima<A, V> &rhs) = default;

    /**
     * Move constructor
     */
    FunctionMaxima(FunctionMaxima<A, V> &&rhs) noexcept = default;

    /**
     * Overloaded assignment operator
     */
    FunctionMaxima &operator=(FunctionMaxima &&rhs) noexcept = default;

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
        point_type toInsert = {a, v};

        auto prevPointIt = pointSet.find(a);
        eraseAdjoinMaxima(prevPointIt);

        erase(a);

        auto currentPointIt = pointSet.insert(toInsert).first;
        eraseAdjoinMaxima(currentPointIt);
        addAdjoinMaxima(currentPointIt);
    }

    void erase(const A &a) {
        point_type toRemove = {a};
        auto it = pointSet.find(toRemove);

        if (it == pointSet.end()) {
            return;
        }

        iterator temp;

        if (it != pointSet.begin()) {
            temp = --it;
        }
        else {
            temp = ++it;
        }

        pointSet.erase(toRemove);

        eraseAdjoinMaxima(temp);
        addAdjoinMaxima(temp);
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

    bool shouldBeMaximum(iterator it) {
        auto tempLesser = it;
        tempLesser--;
        auto tempGreater = it;
        tempGreater++;

        return (it == pointSet.begin() || tempLesser->value() <= it->value()) &&
                (tempGreater == pointSet.end() || tempGreater->value() <= it->value());
    }

    // use this function before inserting point_type to pointSet
    void eraseAdjoinMaxima(iterator it) {
        if (it == pointSet.end()) {
            return;
        }

        auto tempLesser = it;
        tempLesser--;
        auto tempGreater = it;
        tempGreater++;

        if (it != pointSet.begin()) {
            maximaPointSet.erase(*tempLesser);
        }

        maximaPointSet.erase(*it);

        if (tempGreater != pointSet.end()) {
            maximaPointSet.erase(*tempGreater);
        }
    }

    // use this function after inserting point_type to pointSet
    void addAdjoinMaxima(iterator it) {
        auto tempLesser = it;
        tempLesser--;
        auto tempGreater = it;
        tempGreater++;

        if (it != pointSet.begin() && shouldBeMaximum(tempLesser)) {
            maximaPointSet.insert(*tempLesser);
        }

        if (shouldBeMaximum(it)) {
            maximaPointSet.insert(*it);
        }

        if (tempGreater != pointSet.end() && shouldBeMaximum(tempGreater)) {
            maximaPointSet.insert(*tempGreater);
        }
    }

    struct pointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.arg() < b.arg();
        }
    };

    struct maximaPointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.value() > b.value() ||
                (a.value() == b.value() && a.arg() < b.arg());
        }
    };

    std::set<point_type, pointSetCmp> pointSet;
    std::set<point_type, maximaPointSetCmp> maximaPointSet;
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
 * TODO: It may add local maximum in the set of maxima values.
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
 * TODO: It may erase local maximum in the set of maxima values.
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
