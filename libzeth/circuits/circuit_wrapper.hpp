// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_CIRCUITS_CIRCUIT_WRAPPER_HPP__
#define __ZETH_CIRCUITS_CIRCUIT_WRAPPER_HPP__

#include "libzeth/circuits/joinsplit.tcc"
#include "libzeth/core/extended_proof.hpp"
#include "libzeth/core/note.hpp"
#include "libzeth/zeth_constants.hpp"

namespace libzeth
{

/// Wrapper around the joinsplit circuit, using parameterized schemes for
/// hashing, and a snark scheme for generating keys and proofs.
template<
    typename HashT,
    typename HashTreeT,
    typename ppT,
    typename snarkT,
    size_t NumInputs,
    size_t NumOutputs,
    size_t TreeDepth>
class circuit_wrapper
{
public:
    using Field = libff::Fr<ppT>;
    // Both `joinsplit` and `joinsplit_gadget` are already used in the
    // namespace.
    using joinsplit_type = joinsplit_gadget<
        Field,
        HashT,
        HashTreeT,
        NumInputs,
        NumOutputs,
        TreeDepth>;

    circuit_wrapper();
    circuit_wrapper(const circuit_wrapper &) = delete;
    circuit_wrapper &operator=(const circuit_wrapper &) = delete;

    // Generate the trusted setup
    typename snarkT::keypair generate_trusted_setup() const;

    // Retrieve the constraint system (intended for debugging purposes).
    const libsnark::protoboard<Field> &get_constraint_system() const;

    // Generate a proof and returns an extended proof
    extended_proof<ppT, snarkT> prove(
        const Field &root,
        const std::array<joinsplit_input<Field, TreeDepth>, NumInputs> &inputs,
        const std::array<zeth_note, NumOutputs> &outputs,
        const bits64 &vpub_in,
        const bits64 &vpub_out,
        const bits256 &h_sig_in,
        const bits256 &phi_in,
        const typename snarkT::proving_key &proving_key) const;

private:
    libsnark::protoboard<Field> pb;
    std::shared_ptr<joinsplit_type> joinsplit;
};

} // namespace libzeth

#include "libzeth/circuits/circuit_wrapper.tcc"

#endif // __ZETH_CIRCUITS_CIRCUIT_WRAPPER_HPP__
