#pragma once

#include <random>
#include <memory>
#include <assert.h>
#include <type_traits>
#include <functional>

namespace node_rand {

/// \class NodeRNG Base class for generating a random number using a random number generator and distribution
/// \param ParamT Types for distribution [min, max] and return
/// \param Distribution Type for distribution
template<typename ParamT, typename Distribution>
class NodeRNG 
{
    const Distribution m_distribution;

public:
    NodeRNG(const ParamT min, const ParamT max) : m_distribution(min, max) {
        assert(min < max && "Min must be less than Max");
    }

    template<class Generator>
    ParamT Generate(Generator& generator) {
        return static_cast<ParamT>(m_distribution(generator));
    }

    template<class Generator>
    const ParamT Generate(Generator& generator) const {
        return static_cast<ParamT>(m_distribution(generator));
    }
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
 *  const uint64_t result = rng.Generate(generator);
 * 
 *  // Example generating random int8_t using an int16_t distribution bound by int8_t
 *  const int8_t a2 = 0;
 *  const int8_t b2 = 10;
 *  NodeRNGUniformDistribution<int8_t, int16_t> rng2(a2, b2);
 *  const int8_t result2 = rng2.Generate(generator);
 * 
 *  // Example generating random double
 *  const double a3 = 0;
 *  const double b3 = 1;
 *  NodeRNGUniformDistribution rng3(a3, b3);
 *  double result3 = rng3.Generate(generator);
 * 
 */
template<typename ParamT, typename DistT, typename Enable = void>
class NodeRNGUniformDistributionType;

template<typename ParamT, typename DistT>
class NodeRNGUniformDistributionType<ParamT, DistT, std::enable_if_t<std::is_integral<ParamT>::value>> : public NodeRNG<ParamT, std::uniform_int_distribution<DistT>> {
public:
    NodeRNGUniformDistributionType(const ParamT min, const ParamT max) : NodeRNG(min, max) {}
};

template<typename ParamT, typename DistT>
class NodeRNGUniformDistributionType<ParamT, DistT, std::enable_if_t<std::is_floating_point<ParamT>::value>> : public NodeRNG<ParamT, std::uniform_real_distribution<DistT>> {
public:
    NodeRNGUniformDistributionType(const ParamT min, const ParamT max) : NodeRNG(min, max) {}
};

template<typename ParamT, typename DistT = ParamT>
class NodeRNGUniformDistribution : public NodeRNGUniformDistributionType<ParamT, DistT>
{
public:
    NodeRNGUniformDistribution(const ParamT min, const ParamT max) : NodeRNGUniformDistributionType(min, max) {}
};

/**
 * TODO: Other distributions
 *  
 */

}