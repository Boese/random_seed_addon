#pragma once

#include <random>
#include <memory>
#include <assert.h>
#include <type_traits>
#include <functional>

namespace node_rand {

/// \class NodeRNG Base class for generating a random number using a random number generator and distribution
/// \param T Types for distribution [min, max] and return
/// \param Distribution Type of distribution
/// \note Behaves like std distribution types, eg: min(), max(), operator()
template<typename T, typename Distribution>
class NodeRNG {
    const Distribution m_distribution;
public:
    NodeRNG(const T min, const T max) : m_distribution{min, max} {
        assert(max >= min && "Max must be greater or equal to min");
    }

    template<class Generator>
    const T operator()(Generator& generator) {
        return static_cast<T>(const_cast<Distribution&>(m_distribution)(generator));
    }
    const T min() const { return static_cast<T>(m_distribution.min()); }
    const T max() const { return static_cast<T>(m_distribution.max()); }
};

/**
 * Uniform Distribution RNG(s)
 * 
 * Example:
 *  // Example generating random uint64_t
 *  std::mt19937 generator(std::random_device{}());
 *  const uint64_t a = 0;
 *  const uint64_t b = 2000;
 *  NodeRNGUniformDistribution rng(a, b);
 *  const uint64_t result = rng(generator);
 * 
 *  // Example generating random int8_t using an int16_t distribution bound by int8_t
 *  const int8_t a2 = 0;
 *  const int8_t b2 = 10;
 *  NodeRNGUniformDistribution rng2(a2, b2);
 *  const int8_t result2 = rng2(generator);
 * 
 *  // Example generating random double
 *  const double a3 = 0;
 *  const double b3 = 1;
 *  NodeRNGUniformDistribution rng3(a3, b3);
 *  double result3 = rng3(generator);
 * 
 */
template <typename T, typename Enable = void> 
class NodeRNGUniformDistribution
{
public:
    NodeRNGUniformDistribution(const T, const T) {
        assert(false && "Error! Default UnifiormDistribution used. Type T must be Number type");
    }
};

/// \brief Floating point types
template <typename T>
class NodeRNGUniformDistribution<T, std::enable_if_t<std::is_floating_point<T>::value>> : public NodeRNG<T, std::uniform_real_distribution<T>>
{
public:
    NodeRNGUniformDistribution(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max()) : NodeRNG(min, max) {}
};

/// \brief Integer types (non 1 byte size)
template <typename T>
class NodeRNGUniformDistribution<T, std::enable_if_t<std::is_integral<T>::value && (sizeof(T) > 1)>> : public NodeRNG<T, std::uniform_int_distribution<T>> 
{
public:
    NodeRNGUniformDistribution(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max()) : NodeRNG(min, max) {}
};

/// \brief int8_t type (will use int16_t for distribution)
template <typename T>
class NodeRNGUniformDistribution<T, std::enable_if_t<std::is_integral<T>::value
                                                && std::is_same<int8_t, T>::value>> : public NodeRNG<T, std::uniform_int_distribution<int16_t>>
{
public:
    NodeRNGUniformDistribution(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max()) : NodeRNG(min, max) {}
};

/// \brief uint8_t type (will use uint16_t for distribution)
template <typename T>
class NodeRNGUniformDistribution<T, std::enable_if_t<std::is_integral<T>::value
                                                && std::is_same<uint8_t, T>::value>> : public NodeRNG<T, std::uniform_int_distribution<uint16_t>>
{
public:
    NodeRNGUniformDistribution(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max()) : NodeRNG(min, max) {}
};

/**
 * TODO: Other distributions
 *  
 */

}