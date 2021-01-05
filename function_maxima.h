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
     * Impl class will have access to the private parametrized constructors of point_type
     */
    friend class FunctionMaxima<A, V>::Impl;

    // we are making shallow copy of A and V and creating shared_ptr to them
    // shoulnd't we make full copy?
    point_type(A argument, V point) : argument(std::make_shared<A>(argument)), point(std::make_shared<V>(point)) {}

    // we don't need full copy since this object is only temporary
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
            throw InvalidArg("invalid argument value");
        }

        return ((point_type) *it).value();
    }

    void set_value(const A &a, const V &v) {
        Storage storage = {};
        bool insertion = false;

        try {
            point_type toInsert = {a, v};
            storage.surrounding.push_back(pointSet.find(toInsert));

            if(storage.surrounding[prevMiddle] == pointSet.end()) {
                findSurrounding(pointSet.insert(toInsert), storage);
            } else {
                findSurrounding(storage.surrounding[prevMiddle], storage);
                storage.surrounding[newMiddle] = pointSet.insert(toInsert);
            }

            insertion = true;

            updateMaximum(leftmost, left, newMiddle,storage);
            updateMaximum(newMiddle, right, rightmost, storage);
            updateMaximum(left, newMiddle, right, storage);

            if (storage.surrounding[prevMiddle] != pointSet.end()) {
                storage.success.push_back(maximaPointSet.find(*storage.surrounding[prevMiddle]));
            }
        }
        catch (std::exception &e) {
            makeRollback(insertion, storage);

            throw e;
        }

        makeCommit(storage);
    }

    void erase(const A &a) {
        Storage storage = {};

        try {
            point_type toRemove = {a};
            storage.surrounding.push_back(pointSet.find(toRemove));

            if (storage.surrounding[prevMiddle] == pointSet.end()) {
                return;
            }

            findSurrounding(storage.surrounding[prevMiddle], storage);

            updateMaximum(leftmost, left, right, storage);
            updateMaximum(left, right, rightmost, storage);

            if (storage.surrounding[prevMiddle] != pointSet.end()) {
                storage.success.push_back(maximaPointSet.find(*storage.surrounding[prevMiddle]));
            }
        }
        catch (std::exception &e) {
            makeRollback(false, storage);

            throw e;
        }

        makeCommit(storage);
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

    struct Storage {
        Storage() {
            success = std::vector<mx_iterator>();
            rollback = std::vector<mx_iterator>();
            surrounding = std::vector<iterator>();
            success.reserve(requiredSpace);
            rollback.reserve(requiredSpace);
            surrounding.reserve(requiredSpace);
        }

        std::vector<mx_iterator> success;
        std::vector<mx_iterator> rollback;
        std::vector<iterator> surrounding;
    };

    iterator moveItLeft(const iterator it) const {
        auto tempIt = it;

        if (it != pointSet.end() && it != pointSet.begin()) {
            tempIt--;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    iterator moveItRight(const iterator it) const {
        auto tempIt = it;

        if (it != pointSet.end()) {
            tempIt++;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    static bool sameValue(const point_type &p1, const point_type &p2) {
        return (!(p1.value() < p2.value()) && !(p2.value() < p1.value()));
    }

    bool shouldBeMaximum(const iterator leftIt, const iterator it, const iterator rightIt) const {
        return (leftIt == pointSet.end() || leftIt->value() < it->value() ||
                sameValue(*leftIt, *it)) &&
               (rightIt == pointSet.end() || rightIt->value() < it->value() ||
                sameValue(*rightIt, *it));
    }

    void updateMaximum(const size_t left, const size_t middle, const size_t right, Storage &storage) {
        if (storage.surrounding[middle] == pointSet.end()) {
            return;
        }

        auto maximaIt = maximaPointSet.find(*storage.surrounding[middle]);
        bool checkNew = shouldBeMaximum(storage.surrounding[left],
                storage.surrounding[middle], storage.surrounding[right]);

        if (maximaIt != maximaPointSet.end() && !checkNew) {
            storage.success.push_back(maximaIt);
        }

        if (maximaIt == maximaPointSet.end() && checkNew) {
            storage.rollback.push_back(maximaPointSet.insert(*storage.surrounding[middle]));
        }
    }

    void findSurrounding(iterator it, Storage &storage) const {
        storage.surrounding.push_back(it);
        storage.surrounding.push_back(moveItLeft(it));
        storage.surrounding.push_back(moveItRight(it));
        storage.surrounding.push_back(moveItLeft(storage.surrounding[left]));
        storage.surrounding.push_back(moveItRight(storage.surrounding[right]));
    }

    void makeCommit(Storage &storage) {
        for (size_t i = 0; i < storage.success.size(); i++) {
            if (storage.success[i] != maximaPointSet.end()) {
                maximaPointSet.erase(storage.success[i]);
            }
        }

        if (storage.surrounding[prevMiddle] != pointSet.end()) {
            pointSet.erase(storage.surrounding[prevMiddle]);
        }
    }

    void makeRollback(const bool insertion, Storage &storage) {
        for (size_t i = 0; i < storage.rollback.size(); i++) {
            if (storage.rollback[i] != maximaPointSet.end()) {
                maximaPointSet.erase(storage.rollback[i]);
            }
        }

        if (insertion && storage.surrounding[newMiddle] != pointSet.end()) {
            pointSet.erase(storage.surrounding[newMiddle]);
        }
    }

    /**
     * Comparator for the set of all points.
     */
    struct pointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.arg() < b.arg();
        }
    };

    /**
     * Comparator for the set of all maxima points.
     */
    struct maximaPointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return (b.value() < a.value()) ||
                    (sameValue(a, b) && (a.arg() < b.arg()));
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
 * The function will erase the element given by the key.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - const reference to the key of the element to be removed
 */
template<typename A, typename V>
void FunctionMaxima<A, V>::erase(const A &a) {
    return pImpl->erase(a);
}

/**
 * Iteration is done in ascending order according to the keys.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points to the first element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::begin() const {
    return pImpl->begin();
}

/**
 * Iteration is done in ascending order according to the keys.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points one past the last element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::end() const {
    return pImpl->end();
}

/**
 * @brief tries to locate an element in FunctionMaxima.
 *
 *  This function takes a key and tries to locate the element with which
 *  the argument matches.  If successful the function returns an iterator
 *  pointing to the sought after element. If unsuccessful it returns the
 *  past-the-end ( @c end() ) iterator.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @param a - element to be located
 * @return iterator pointing to sought-after element, or end() if not found.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::find(const A &a) const {
    return pImpl->find(a);
}

/**
 * Iteration is done in descending order according to the values.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points to the first maxima element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_begin() const {
    return pImpl->mx_begin();
}

/**
 * Iteration is done in descending order according to the values.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points one past the last maxima element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_end() const {
    return pImpl->mx_end();
}

/**
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return the size of the domain of FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const {
    return pImpl->size();
}

#endif //MAXIMA_FUNCTION_MAXIMA_H
