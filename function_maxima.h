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

    class iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = point_type;
        using pointer = point_type *;
        using reference = point_type;

        iterator(const typename std::set<point_type>::iterator &ptr) : ptr(ptr) {}

        reference operator*() const { return *ptr; }

        pointer operator->() { return ptr; }

        iterator &operator++() {
            ptr++;

            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);

            return tmp;
        }

        friend bool operator==(const iterator &a, const iterator &b) {
            return a.ptr == b.ptr;
        }

        friend bool operator!=(const iterator &a, const iterator &b) {
            return a.ptr != b.ptr;
        }

    private:
        typename std::set<point_type>::iterator ptr;
    };

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
        return argument;
    }

    V const &value() const {
        return point;
    }

    point_type(const point_type &rhs) : argument(rhs.argument), point(rhs.point) {
    }

    point_type(point_type &&rhs) noexcept: argument(std::move(rhs.argument)), point(std::move(rhs.point)) {
    }

private:
    /**
     * Impl class will have access to the private parametrized constructor of point_type
     */
    friend class FunctionMaxima<A, V>::Impl;

    point_type(A argument, V point) : argument(argument), point(point) {}

    A argument;
    V point;
};

/*********************************FUNCTION_MAXIMA_IMPL*********************************/

template<typename A, typename V>
class FunctionMaxima<A, V>::Impl {
public:
    Impl() = default;

    /**
     * TODO: This must be changed
     */
    V const &value_at(const A &a) const {
        V v;
        point_type toSearch = {a, v};

        auto it = pointSet.find(toSearch);

        if (it == pointSet.end()) {
            throw InvalidArg("The argument is out of the domain.");
        }

        return ((point_type) *it).value();
    }

    void set_value(const A &a, const V &v) {
        erase(a);
        point_type toInsert = {a, v};

        pointSet.insert(toInsert);
    }

    /**
     * TODO: This must be changed
     */
    void erase(const A &a) {
        V v;
        point_type toRemove = {a, v};

        pointSet.erase(toRemove);
    }

    FunctionMaxima<A, V>::iterator begin() const {
        return pointSet.begin();
    }

    FunctionMaxima<A, V>::iterator end() const {
        return pointSet.end();
    }

    FunctionMaxima<A, V>::iterator find(A const &a) const {
        V v;
        point_type toSearch = {a, v};

        return pointSet.find(toSearch);
    }

    size_type size() const {
        return pointSet.size();
    }


private:
    struct pointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.argument < b.argument;
        }
    };

    std::set<point_type, pointSetCmp> pointSet;
    std::set<point_type> maximaPointSet;
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
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const {
    return pImpl->size();
}

#endif //MAXIMA_FUNCTION_MAXIMA_H
