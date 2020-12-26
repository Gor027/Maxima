//
// Created by gor027 on 15.12.2020.
//

#ifndef MAXIMA_FUNCTION_MAXIMA_H
#define MAXIMA_FUNCTION_MAXIMA_H

#include <map>
#include <set>
#include <vector>

/****************************InvalidArgException***********************************/

class InvalidArg : public std::exception {
public:
    explicit InvalidArg(const char *msg) : errorMsg_(msg) {}

    virtual const char *what() const noexcept {
        return errorMsg_;
    }

private:
    const char *errorMsg_;
};

template<typename A, typename V>
class FunctionMaxima {
public:
    class point_type;

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
        pointSet = nullptr;
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

    // TODO:
    typename std::set<point_type>::iterator find(A const &a) const {
        return;
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
}

/**
 * The function will update the value of the existing key.
 * If the key does not exist in the map, then it will be inserted
 * with the value assigned to it.
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
}

/**
 * The function erases the element given by the key.
 * Note that if the element is a pointer then the pointed-to memory will not be touched.
 *
 * TODO: It may erase local maximum in the set of maxima values.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - const reference to the key of the element to be removed
 */
template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
}

/****************************POINTER_TYPE***********************************/

template<typename A, typename V>
class FunctionMaxima<A, V>::point_type {
public:
    point_type(A argument, V point) : argument(argument), point(point) {}

    A const &arg() const {
        return argument;
    }

    V const &value() const {
        return point;
    }

    /**
     *
     * Note: The comparing operator may throw an exception.
     *
     * @param rhs
     * @return
     */
    bool operator<(const FunctionMaxima<A, V>::point_type &rhs) {
        return this->argument < rhs.argument;
    }

    point_type(const point_type &rhs) : argument(rhs.argument), point(rhs.point) {
    }

    point_type(point_type &&rhs) noexcept: argument(std::move(rhs.argument)), point(std::move(rhs.point)) {
    }

private:
    A argument;
    V point;
};

#endif //MAXIMA_FUNCTION_MAXIMA_H
