//
// Created by gor027 on 15.12.2020.
//

#ifndef MAXIMA_FUNCTION_MAXIMA_H
#define MAXIMA_FUNCTION_MAXIMA_H

#include <map>
#include <set>
#include <vector>

class InvalidArg : public std::exception {
public:
    explicit InvalidArg(const char *message) : errorMessage(message) {}

    virtual const char *what() const noexcept {
        return errorMessage;
    }

private:
    const char *errorMessage;
};

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type {
    public:
        A const &arg() const {
            return argument;
        }

        V const &value() const {
            return point;
        }

        /**
         * TODO: Fix the operator
         * Note: The comparing operator may throw an exception.
         *
         * @param rhs
         * @return
         */
        bool operator<(const FunctionMaxima<A, V>::point_type &rhs) const {
            return (this->argument < rhs.argument);
//            || (!(this->argument < rhs.argument) && !(rhs.argument < this->argument) && this->point < rhs.point);
        }

        point_type(const point_type &rhs) : argument(rhs.argument), point(rhs.point) {
        }

        point_type(point_type &&rhs) noexcept: argument(std::move(rhs.argument)), point(std::move(rhs.point)) {
        }

        /**
         * TODO: Hide the constructor from outer world
         * @param argument
         * @param point
         */
        point_type(A argument, V point) : argument(argument), point(point) {}

    private:
        A argument;
        V point;
    };

    using size_type = std::size_t;

    explicit FunctionMaxima() = default;

    /**
     * Copy constructor
     */
    FunctionMaxima(const FunctionMaxima<A, V> &rhs) : pointSet(rhs.pointSet) {
    }

    /**
     * Move constructor
     */
    FunctionMaxima(FunctionMaxima<A, V> &&rhs) noexcept: pointSet(std::move(rhs.pointSet)) {
        rhs.pointSet = nullptr;
    }

    /**
     * Overloaded assignment operator
     */
    FunctionMaxima &operator=(FunctionMaxima &&rhs) noexcept {
        pointSet = std::move(rhs.pointSet);

        return *this;
    }

    ~FunctionMaxima() = default;

    V const &value_at(A const &a) const;

    void set_value(A const &a, V const &v);

    void erase(A const &a);

    typename std::set<point_type>::iterator begin() const {
        return pointSet.begin();
    }

    typename std::set<point_type>::iterator end() const {
        return pointSet.end();
    }

    typename std::set<point_type>::iterator find(A const &a) const {
    }

    size_type size() const {
        return pointSet.size();
    }

private:
    std::set<point_type> pointSet;
};

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
    V v;
    FunctionMaxima<A, V>::point_type toSearch = {a, v};
    auto it = pointSet.find(toSearch);

    if (it == pointSet.end()) {
        throw InvalidArg("The argument is not in the range of the domain.");
    }

    return ((FunctionMaxima<A, V>::point_type) *it).value();
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
    erase(a);
    FunctionMaxima<A, V>::point_type toInsert = {a, v};
    pointSet.insert(toInsert);
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
    V v;
    FunctionMaxima<A, V>::point_type toRemove = {a, v};
    pointSet.erase(toRemove);
}

#endif //MAXIMA_FUNCTION_MAXIMA_H
