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
    const ParamT Generate(Generator& generator) const {
        return static_cast<ParamT>(m_distribution(generator));
    }
};

/**
 * Uniform Distribution RNG(S)
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