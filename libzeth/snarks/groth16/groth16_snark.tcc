// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_SNARKS_GROTH16_GROTH16_SNARK_TCC__
#define __ZETH_SNARKS_GROTH16_GROTH16_SNARK_TCC__

#include "libzeth/core/group_element_utils.hpp"
#include "libzeth/core/utils.hpp"
#include "libzeth/serialization/r1cs_serialization.hpp"
#include "libzeth/snarks/groth16/groth16_snark.hpp"

namespace libzeth
{

template<typename ppT> const std::string groth16_snark<ppT>::name("GROTH16");

template<typename ppT>
typename groth16_snark<ppT>::keypair groth16_snark<ppT>::generate_setup(
    const libsnark::protoboard<libff::Fr<ppT>> &pb)
{
    // Generate verification and proving key from the R1CS
    return libsnark::r1cs_gg_ppzksnark_generator<ppT>(
        pb.get_constraint_system(), true);
}

template<typename ppT>
typename groth16_snark<ppT>::proof groth16_snark<ppT>::generate_proof(
    const typename groth16_snark<ppT>::proving_key &proving_key,
    const libsnark::protoboard<libff::Fr<ppT>> &pb)
{
    return generate_proof(
        proving_key, pb.primary_input(), pb.auxiliary_input());
}

template<typename ppT>
typename groth16_snark<ppT>::proof groth16_snark<ppT>::generate_proof(
    const proving_key &proving_key,
    const libsnark::r1cs_primary_input<libff::Fr<ppT>> &primary_input,
    const libsnark::r1cs_auxiliary_input<libff::Fr<ppT>> auxiliary_input)
{
    // Generate proof from public input, auxiliary input and proving key.
    // For now, force a pow2 domain, in case the key came from the MPC.
    return libsnark::r1cs_gg_ppzksnark_prover(
        proving_key, primary_input, auxiliary_input, true);
}

template<typename ppT>
bool groth16_snark<ppT>::verify(
    const libsnark::r1cs_primary_input<libff::Fr<ppT>> &primary_inputs,
    const groth16_snark<ppT>::proof &proof,
    const groth16_snark<ppT>::verification_key &verification_key)
{
    return libsnark::r1cs_gg_ppzksnark_verifier_strong_IC<ppT>(
        verification_key, primary_inputs, proof);
}

template<typename ppT>
void groth16_snark<ppT>::verification_key_write_json(
    const verification_key &vk, std::ostream &out_s)
{
    const size_t abc_length = vk.ABC_g1.rest.indices.size() + 1;
    out_s << "{"
          << "\n"
          << "  \"alpha\": " << group_element_to_json(vk.alpha_g1) << ",\n"
          << "  \"beta\": " << group_element_to_json(vk.beta_g2) << ",\n"
          << "  \"delta\": " << group_element_to_json(vk.delta_g2) << ",\n"
          << "  \"ABC\": [\n    " << group_element_to_json(vk.ABC_g1.first);
    for (size_t i = 1; i < abc_length; ++i) {
        out_s << ",\n    "
              << group_element_to_json(vk.ABC_g1.rest.values[i - 1]);
    }
    out_s << "\n  ]\n}";
}

template<typename ppT>
void groth16_snark<ppT>::verification_key_write_bytes(
    const verification_key &vk, std::ostream &out_s)
{
    using G1 = libff::G1<ppT>;

    if (!is_well_formed<ppT>(vk)) {
        throw std::invalid_argument("verification key (write) not well-formed");
    }

    group_element_write_bytes(vk.alpha_g1, out_s);
    group_element_write_bytes(vk.beta_g2, out_s);
    group_element_write_bytes(vk.delta_g2, out_s);
    accumulation_vector_write_bytes<G1, group_element_write_bytes<G1>>(
        vk.ABC_g1, out_s);
}

template<typename ppT>
void groth16_snark<ppT>::proving_key_write_bytes(
    const proving_key &pk, std::ostream &out_s)
{
    if (!is_well_formed<ppT>(pk)) {
        throw std::invalid_argument("proving key (write) not well-formed");
    }

    group_element_write_bytes(pk.alpha_g1, out_s);
    group_element_write_bytes(pk.beta_g1, out_s);
    group_element_write_bytes(pk.beta_g2, out_s);
    group_element_write_bytes(pk.delta_g1, out_s);
    group_element_write_bytes(pk.delta_g2, out_s);
    group_elements_write_bytes(pk.A_query, out_s);
    knowledge_commitment_vector_write_bytes(pk.B_query, out_s);
    group_elements_write_bytes(pk.H_query, out_s);
    group_elements_write_bytes(pk.L_query, out_s);
    r1cs_write_bytes(pk.constraint_system, out_s);
}

template<typename ppT>
void groth16_snark<ppT>::verification_key_read_bytes(
    groth16_snark<ppT>::verification_key &vk, std::istream &in_s)
{
    using G1 = libff::G1<ppT>;

    group_element_read_bytes(vk.alpha_g1, in_s);
    group_element_read_bytes(vk.beta_g2, in_s);
    group_element_read_bytes(vk.delta_g2, in_s);
    accumulation_vector_read_bytes<G1, group_element_read_bytes<G1>>(
        vk.ABC_g1, in_s);

    if (!is_well_formed<ppT>(vk)) {
        throw std::invalid_argument("verification key (read) not well-formed");
    }
}

template<typename ppT>
void groth16_snark<ppT>::proving_key_read_bytes(
    groth16_snark<ppT>::proving_key &pk, std::istream &in_s)
{
    group_element_read_bytes(pk.alpha_g1, in_s);
    group_element_read_bytes(pk.beta_g1, in_s);
    group_element_read_bytes(pk.beta_g2, in_s);
    group_element_read_bytes(pk.delta_g1, in_s);
    group_element_read_bytes(pk.delta_g2, in_s);
    group_elements_read_bytes(pk.A_query, in_s);
    knowledge_commitment_vector_read_bytes(pk.B_query, in_s);
    group_elements_read_bytes(pk.H_query, in_s);
    group_elements_read_bytes(pk.L_query, in_s);
    r1cs_read_bytes(pk.constraint_system, in_s);

    if (!is_well_formed<ppT>(pk)) {
        throw std::invalid_argument("proving key (read) not well-formed");
    }
}

template<typename ppT>
void groth16_snark<ppT>::proof_write_json(
    const typename groth16_snark<ppT>::proof &proof, std::ostream &out_s)
{
    out_s << "{\n  \"a\": " << group_element_to_json(proof.g_A)
          << ",\n  \"b\": " << group_element_to_json(proof.g_B)
          << ",\n  \"c\": " << group_element_to_json(proof.g_C) << "\n}";
}

template<typename ppT>
void groth16_snark<ppT>::proof_write_bytes(
    const typename groth16_snark<ppT>::proof &proof, std::ostream &out_s)
{
    group_element_write_bytes(proof.g_A, out_s);
    group_element_write_bytes(proof.g_B, out_s);
    group_element_write_bytes(proof.g_C, out_s);
}

template<typename ppT>
void groth16_snark<ppT>::proof_read_bytes(
    typename groth16_snark<ppT>::proof &proof, std::istream &in_s)
{
    group_element_read_bytes(proof.g_A, in_s);
    group_element_read_bytes(proof.g_B, in_s);
    group_element_read_bytes(proof.g_C, in_s);
}

template<typename ppT>
void groth16_snark<ppT>::keypair_write_bytes(
    const typename groth16_snark<ppT>::keypair &keypair, std::ostream &out_s)
{
    proving_key_write_bytes(keypair.pk, out_s);
    verification_key_write_bytes(keypair.vk, out_s);
}

template<typename ppT>
void groth16_snark<ppT>::keypair_read_bytes(
    typename groth16_snark<ppT>::keypair &keypair, std::istream &in_s)
{
    proving_key_read_bytes(keypair.pk, in_s);
    verification_key_read_bytes(keypair.vk, in_s);
}

template<typename ppT>
bool is_well_formed(const typename groth16_snark<ppT>::proving_key &pk)
{
    if (!pk.alpha_g1.is_well_formed() || !pk.beta_g1.is_well_formed() ||
        !pk.beta_g2.is_well_formed() || !pk.delta_g1.is_well_formed() ||
        !pk.delta_g2.is_well_formed() ||
        !container_is_well_formed(pk.A_query) ||
        !container_is_well_formed(pk.L_query)) {
        return false;
    }

    using knowledge_commitment =
        libsnark::knowledge_commitment<libff::G2<ppT>, libff::G1<ppT>>;
    for (const knowledge_commitment &b : pk.B_query.values) {
        if (!b.g.is_well_formed() || !b.h.is_well_formed()) {
            return false;
        }
    }

    return true;
}

template<typename ppT>
bool is_well_formed(const typename groth16_snark<ppT>::verification_key &vk)
{
    if (!vk.alpha_g1.is_well_formed() || !vk.beta_g2.is_well_formed() ||
        !vk.delta_g2.is_well_formed() || !vk.ABC_g1.first.is_well_formed()) {
        return false;
    }

    return container_is_well_formed(vk.ABC_g1.rest.values);
}

} // namespace libzeth

#endif // __ZETH_SNARKS_GROTH16_GROTH16_SNARK_TCC__
