#ifndef MAXIMA_FUNCTION_MAXIMA_H
#define MAXIMA_FUNCTION_MAXIMA_H

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <memory>
#include <iterator>

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

    using iterator = typename std::multiset<point_type>::iterator;
    using mx_iterator = typename std::multiset<point_type>::iterator;

    explicit FunctionMaxima();

    /**
     * Copy constructors
     */
    FunctionMaxima(const FunctionMaxima<A, V> &rhs) : pImpl(std::make_unique<Impl>(*rhs.pImpl)) {}

    FunctionMaxima &operator=(const FunctionMaxima &rhs) {
        pImpl = std::make_unique<Impl>(*rhs.pImpl);

        return *this;
    }

    /**
     * Move constructors
     */
    FunctionMaxima(FunctionMaxima<A, V> &&rhs) noexcept = default;

    FunctionMaxima &operator=(FunctionMaxima &&rhs) noexcept = default;

    /**
     * Destructor
     */
    ~FunctionMaxima() = default;

    V const &value_at(A const &a) const;

    void set_value(A const &a, V const &v);

    void erase(A const &a);

    iterator begin() const noexcept;

    iterator end() const noexcept;

    iterator find(A const &a) const;

    mx_iterator mx_begin() const noexcept;

    mx_iterator mx_end() const noexcept;

    size_type size() const noexcept;

private:
    class Impl;

    std::unique_ptr<Impl> pImpl;
};

/*********************************POINT_TYPE*********************************/

template<typename A, typename V>
class FunctionMaxima<A, V>::point_type {
public:
    A const &arg() const noexcept {
        return *argument.get();
    }

    V const &value() const noexcept {
        return *point.get();
    }

    point_type(const point_type &rhs) = default;

private:
    /**
     * Impl class will have access to the private parametrized constructors of point_type
     */
    friend class FunctionMaxima<A, V>::Impl;

    point_type(const A &argument, const V &point) : argument(std::make_shared<A>(argument)),
                                                    point(std::make_shared<V>(point)) {}

    point_type(const A &argument) : argument(std::make_shared<A>(argument)), point(nullptr) {}

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

            if (storage.surrounding[prevMiddle] == pointSet.end()) {
                findSurrounding(pointSet.insert(toInsert), storage);
            } else {
                if (sameValue(toInsert, *storage.surrounding[prevMiddle])) {
                    return;
                }

                findSurrounding(storage.surrounding[prevMiddle], storage);
                storage.surrounding[newMiddle] = pointSet.insert(toInsert);
            }

            insertion = true;

            updateMaximum(leftmost, left, newMiddle, storage);
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
        catch (...) {
            makeRollback(insertion, storage);

            throw;
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
        catch (...) {
            makeRollback(false, storage);

            throw;
        }

        makeCommit(storage);
    }

    FunctionMaxima<A, V>::iterator begin() const noexcept {
        return pointSet.begin();
    }

    FunctionMaxima<A, V>::iterator end() const noexcept {
        return pointSet.end();
    }

    FunctionMaxima<A, V>::iterator find(A const &a) const {
        point_type toSearch = {a};

        return pointSet.find(toSearch);
    }

    FunctionMaxima<A, V>::mx_iterator mx_begin() const noexcept {
        return maximaPointSet.begin();
    }

    FunctionMaxima<A, V>::mx_iterator mx_end() const noexcept {
        return maximaPointSet.end();
    }

    size_type size() const noexcept {
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
        requiredSpace
    };

    /**
     * Struct which contains std::vector's with reserved necessary amount of space
     * for operations in Implementation.
     * It makes push_back() nothrow (on those vectors).
     */
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

    /**
     * Function is nothrow because it only tries to move given iterator using iterator arithmetic.
     *
     * @param it - iterator
     * @return   - it-- (in terms of iterator arithmetic) or pointSet.end() if it-- is out of set's range.
     */
    iterator moveItLeft(const iterator it) const noexcept {
        auto tempIt = it;

        if (it != pointSet.end() && it != pointSet.begin()) {
            tempIt--;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    /**
     * Function is nothrow because it only tries to move given iterator using iterator arithmetic.
     *
     * @param it - iterator
     * @return   - it++ (in terms of iterator arithmetic) or pointSet.end() if it++ is out of set's range.
     */
    iterator moveItRight(const iterator it) const noexcept {
        auto tempIt = it;

        if (it != pointSet.end()) {
            tempIt++;
        } else {
            tempIt = pointSet.end();
        }

        return tempIt;
    }

    /**
     * Function has strong guarantee:
     * comparing point_types has strong guarantee.
     *
     * @param p1 - left point_type operand
     * @param p2 - right point_type operand
     * @return   - true if the given point_type operands are the same, otherwise false.
     */
    static bool sameValue(const point_type &p1, const point_type &p2) {
        return (!(p1.value() < p2.value()) && !(p2.value() < p1.value()));
    }

    /**
     * Function has strong guarantee because it only uses functions with at least strong guarantee:
     * comparing point_type objects has strong guarantee.
     *
     * @param leftIt  - iterator pointing to a point_type object that is lesser
     *                  and is the closest (in terms of comparing arguments) to *it in pointSet
     * @param it      - iterator pointing to a point_type object that may be maxima
     * @param rightIt - iterator pointing to a point_type object that is greater
     *                  and is the closest (in terms of comparing arguments) to *it in pointSet
     * @return        - true if it points to a point_type object that is maxima, otherwise false.
     */
    bool shouldBeMaximum(const iterator leftIt, const iterator it, const iterator rightIt) const {
        return (leftIt == pointSet.end() || leftIt->value() < it->value() ||
                sameValue(*leftIt, *it)) &&
               (rightIt == pointSet.end() || rightIt->value() < it->value() ||
                sameValue(*rightIt, *it));
    }

    /**
     * Updates maximaPointSet by checking whether point described by middle should be
     * new maxima and inserts it to the maximaPointSet.
     * Also updates iterators that should be erased in case of success and rollback
     * (which are stored in storage).
     * Function has strong guarantee because it only uses functions with at least strong guarantee:
     * find() or insert() on std::multiset<point_type> where comparing point_type objects has strong guarantee,
     * push_back() on std::vector is nothrow if the vector has enough reserved space which is the case here.
     *
     * @param left    - description of point that is lesser and is the closest (in terms of comparing arguments) to *it in pointSet
     * @param middle  - description of point that is being updated
     * @param right   - description of point that is greater and is the closest (in terms of comparing arguments) to *it in pointSet
     * @param storage - struct containing necessary data
     */
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

    /**
     * Updates content of storage with iterators of neighbours of the middle point.
     * Function is nothrow because it uses only nothrow functions:
     * push_back() on std::vector is nothrow if the vector has enough reserved space which is the case here.
     *
     * @param it      - iterator to the middle point
     * @param storage - struct containing necessary data
     */
    void findSurrounding(iterator it, Storage &storage) const noexcept {
        storage.surrounding.push_back(it);
        storage.surrounding.push_back(moveItLeft(it));
        storage.surrounding.push_back(moveItRight(it));
        storage.surrounding.push_back(moveItLeft(storage.surrounding[left]));
        storage.surrounding.push_back(moveItRight(storage.surrounding[right]));
    }

    /**
     * Makes commit in form of erasing outdated points from maximaPointSet
     * (their iterators are stored in storage)
     * and erases outdated point from pointSet (is such one exists).
     * Function is nothrow:
     * erase on std::multiset<point_type> by iterator is nothrow.
     *
     * @param insertion - bool determining whether insertion to pointSet was made
     * @param storage   - struct containing necessary data
     */
    void makeCommit(Storage &storage) noexcept {
        for (size_t i = 0; i < storage.success.size(); i++) {
            if (storage.success[i] != maximaPointSet.end()) {
                maximaPointSet.erase(storage.success[i]);
            }
        }

        if (storage.surrounding[prevMiddle] != pointSet.end()) {
            pointSet.erase(storage.surrounding[prevMiddle]);
        }
    }

    /**
     * Makes rollback in form of erasing inserted points from maximaPointSet
     * (their iterators are stored in storage)
     * and erases inserted point from pointSet (is such one exists).
     * Function is nothrow:
     * erase on std::multiset<point_type> by iterator is nothrow.
     *
     * @param insertion - bool determining whether insertion to pointSet was made
     * @param storage   - struct containing necessary data
     */
    void makeRollback(const bool insertion, Storage &storage) noexcept {
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
     * Comparator for the multiset of all points.
     */
    struct pointSetCmp {
        bool operator()(point_type a, point_type b) const {
            return a.arg() < b.arg();
        }
    };

    /**
     * Comparator for the multiset of all maxima points.
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
 * The function will look up for the key in the multiset.
 * In case there is no such key, Invalid Argument exception is thrown.
 * Function has strong guarantee:
 * find() on std::multiset<point_type> (where comparing point_type objects has strong guarantee) has strong guarantee.
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
 * If the key does not exist in the multiset, then it will be inserted
 * with the value assigned to it.
 * Updates maximaPointSet is necessary.
 * Function has strong guarantee because:
 * push_back() on std::vector stored in storage is nothrow,
 * find() and insert() on std::multiset<point_type> have strong guarantee.
 * First it tries to do all the inserts (strong guarantee) and at the end it erases by iterator (nothrow).
 * When exception is thrown, the function erases all inserts made during it's performance by iterators (nothrow).
 * Those actions assure that the function has strong guarantee.
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
 * Function has strong guarantee because:
 * push_back() on std::vector stored in storage is nothrow,
 * find() and insert() on std::multiset<point_type> has strong guarantee.
 * First it tries to do all the inserts (strong guarantee) and at the end it erases by iterator (nothrow).
 * When exception is thrown, the function erases all inserts made during it's performance by iterators (nothrow).
 * Those actions assure that function has strong guarantee.
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
 * Function is nothrow because begin() on std::multiset is nothrow.
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points to the first element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::begin() const noexcept {
    return pImpl->begin();
}

/**
 * Iteration is done in ascending order according to the keys.
 * Function is nothrow because end() on std::multiset is nothrow.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points one past the last element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::iterator FunctionMaxima<A, V>::end() const noexcept {
    return pImpl->end();
}

/**
 *  This function takes a key and tries to locate the element with which
 *  the argument matches.  If successful the function returns an iterator
 *  pointing to the sought after element. If unsuccessful it returns the
 *  past-the-end ( @c end() ) iterator.
 *  Function has strong guarantee:
 *  find() on std::multiset<point_type> (where comparing point_type objects has strong guarantee) has strong guarantee.
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
 * Function is nothrow because begin() on std::multiset is nothrow.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points to the first maxima element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_begin() const noexcept {
    return pImpl->mx_begin();
}

/**
 * Iteration is done in descending order according to the values.
 * Function is nothrow because end() on std::multiset is nothrow.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return a read-only (constant) iterator that points one past the last maxima element in FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::mx_iterator FunctionMaxima<A, V>::mx_end() const noexcept {
    return pImpl->mx_end();
}

/**
 * Function is nothrow because size() on std::multiset is nothrow.
 *
 * @tparam A - type of the domain values
 * @tparam V - type of the range values
 * @return the size of the domain of FunctionMaxima.
 */
template<typename A, typename V>
typename FunctionMaxima<A, V>::size_type FunctionMaxima<A, V>::size() const noexcept {
    return pImpl->size();
}

#endif //MAXIMA_FUNCTION_MAXIMA_H
