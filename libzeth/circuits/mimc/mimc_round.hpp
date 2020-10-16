// DISCLAIMER:
// Content taken and adapted from:
// https://github.com/HarryR/ethsnarks/blob/master/src/gadgets/mimc.hpp

#ifndef __ZETH_CIRCUITS_MIMC_ROUND_HPP__
#define __ZETH_CIRCUITS_MIMC_ROUND_HPP__

#include "libzeth/circuits/circuit_utils.hpp"

#include <libsnark/gadgetlib1/gadget.hpp>

namespace libzeth
{

// TODO: refactor so that the caller allocates the result, in line with other
// gadgets. The MiMC_mp_gadget should then be able to eliminate an unncessary
// equality constraint for the MP part, by passing in a linear combination for
// the result.

/// MiMCe7_round_gadget enforces correct computation of a MiMC permutation round
/// with exponent 7. In MiMC permutation last round differs from the others
/// since the key is added again. We use a boolean variable `add_k_to_result` to
/// manage this case.
template<typename FieldT>
class MiMCe7_round_gadget : public libsnark::gadget<FieldT>
{
private:
    // Message of the current round
    const libsnark::pb_variable<FieldT> x;
    // Key of the current round
    const libsnark::pb_variable<FieldT> k;
    // Round constant of the current round
    const FieldT c;
    // Boolean variable to add the key after the round
    const bool add_k_to_result;

    // Intermediary var for computing t**2
    libsnark::pb_variable<FieldT> t2;
    // Intermediary var for computing t**4
    libsnark::pb_variable<FieldT> t4;
    // Intermediary var for computing t**6
    libsnark::pb_variable<FieldT> t6;
    // Intermediary result for computing t**7 (or t ** 7 + k depending on
    // add_k_to_result)
    libsnark::pb_variable<FieldT> t7;

public:
    MiMCe7_round_gadget(
        libsnark::protoboard<FieldT> &pb,
        // Message of the current round
        const libsnark::pb_variable<FieldT> &x,
        // Key of the current round
        const libsnark::pb_variable<FieldT> &k,
        // Round constant of the current round
        const FieldT &c,
        // Boolean variable to add the key after the round
        const bool add_k_to_result,
        const std::string &annotation_prefix = "MiMCe7_round_gadget");

    void generate_r1cs_constraints();
    void generate_r1cs_witness() const;

    // Returns round result t ** 7 + add_k_to_result * k
    // where t = (x + k + c)
    const libsnark::pb_variable<FieldT> &result() const;
};

/// MiMCe31_round_gadget enforces correct computation of a MiMC permutation
/// round with exponent 31. `key` is optionally added to the output (based on
/// the value of `add_k_to_result`), to handle the last round.
template<typename FieldT>
class MiMCe31_round_gadget : public libsnark::gadget<FieldT>
{
private:
    const libsnark::pb_variable<FieldT> x;
    const libsnark::pb_variable<FieldT> k;
    const FieldT c;
    const bool add_k_to_result;

    // Intermediate variables
    libsnark::pb_variable<FieldT> t2;
    libsnark::pb_variable<FieldT> t4;
    libsnark::pb_variable<FieldT> t8;
    libsnark::pb_variable<FieldT> t16;
    libsnark::pb_variable<FieldT> t24;
    libsnark::pb_variable<FieldT> t28;
    libsnark::pb_variable<FieldT> t30;
    // Result variable
    libsnark::pb_variable<FieldT> t31;

public:
    MiMCe31_round_gadget(
        libsnark::protoboard<FieldT> &pb,
        const libsnark::pb_variable<FieldT> &x,
        const libsnark::pb_variable<FieldT> &k,
        const FieldT &c,
        const bool add_k_to_result,
        const std::string &annotation_prefix = "MiMCe31_round_gadget");

    void generate_r1cs_constraints();
    void generate_r1cs_witness() const;

    const libsnark::pb_variable<FieldT> &result() const;
};

/// MiMCe11_round_gadget enforces correct computation of a MiMC permutation
/// round with exponent 11. `key` is optionally added to the output (based on
/// the value of `add_k_to_result`), to handle the last round.
template<typename FieldT>
class MiMCe11_round_gadget : public libsnark::gadget<FieldT>
{
private:
    const libsnark::pb_variable<FieldT> x;
    const libsnark::pb_variable<FieldT> k;
    const FieldT c;
    const bool add_k_to_result;

    // Intermediate variables
    libsnark::pb_variable<FieldT> t2;
    libsnark::pb_variable<FieldT> t4;
    libsnark::pb_variable<FieldT> t8;
    libsnark::pb_variable<FieldT> t10;
    // Result variable
    libsnark::pb_variable<FieldT> t11;

public:
    MiMCe11_round_gadget(
        libsnark::protoboard<FieldT> &pb,
        const libsnark::pb_variable<FieldT> &x,
        const libsnark::pb_variable<FieldT> &k,
        const FieldT &c,
        const bool add_k_to_result,
        const std::string &annotation_prefix = "MiMCe11_round_gadget");

    void generate_r1cs_constraints();
    void generate_r1cs_witness() const;

    const libsnark::pb_variable<FieldT> &result() const;
};

} // namespace libzeth

#include "libzeth/circuits/mimc/mimc_round.tcc"

#endif // __ZETH_CIRCUITS_MIMC_ROUND_HPP__
