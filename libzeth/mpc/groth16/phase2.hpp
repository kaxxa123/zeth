// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#ifndef __ZETH_MPC_GROTH16_PHASE2_HPP__
#define __ZETH_MPC_GROTH16_PHASE2_HPP__

#include "libzeth/mpc/groth16/mpc_hash.hpp"
#include "libzeth/snarks/groth16/groth16_snark.hpp"

// Structures and operations related to the "Phase 2" MPC described in
// [BoweGM17].  Parts of the implementation use techniques from the
// "Phase2" library from "zk-SNARK MPCs, made easy".
//
// References:
//
// \[BoweGM17]
//  "Scalable Multi-party Computation for zk-SNARK Parameters in the Random
//  Beacon Model"
//  Sean Bowe and Ariel Gabizon and Ian Miers,
//  IACR Cryptology ePrint Archive 2017,
//  <http://eprint.iacr.org/2017/1050>
//
// "Phase2" (From "zk-SNARK MPCs, made easy" library
//  https://github.com/ebfull/phase2)
//
// "Sapling MPC" ("Multi-party computation for Zcash's Sapling zk-SNARK public
//  parameters" https://github.com/zcash-hackworks/sapling-mpc)

namespace libzeth
{

template<typename ppT> class srs_powersoftau;
template<typename ppT> class srs_mpc_layer_L1;

/// Target of the MPC for Phase2 of the SRS generation.  Follows exactly $M_2$
/// in section 7.3 of [BoweGM17], whre we use L in place of K, consistent with
/// the keypair in libsnark.
///
/// Implements the interfaces of StructuredT and ReadableT templates.
template<typename ppT> class srs_mpc_phase2_accumulator
{
public:
    // Hash of the initial state before any contributions are made. Kept
    // constant over the MPC, and used to check that challenges and responses
    // are part of the same MPC.
    mpc_hash_t cs_hash;

    libff::G1<ppT> delta_g1;

    libff::G2<ppT> delta_g2;

    // { H_i } = { [ t(x) . x^i / delta ]_1 }  i \in [0..n-2] (n-1 entries)
    libff::G1_vector<ppT> H_g1;

    // { L_j } = { [ ABC_j / delta ]_1 } j \in [num_inputs + 1..num_variables]
    libff::G1_vector<ppT> L_g1;

    srs_mpc_phase2_accumulator(
        const mpc_hash_t cs_hash,
        const libff::G1<ppT> &delta_g1,
        const libff::G2<ppT> &delta_g2,
        libff::G1_vector<ppT> &&H_g1,
        libff::G1_vector<ppT> &&L_g1);

    bool operator==(const srs_mpc_phase2_accumulator<ppT> &other) const;
    bool is_well_formed() const;
    void write(std::ostream &out) const;
    void write_compressed(std::ostream &out) const;
    static srs_mpc_phase2_accumulator<ppT> read(std::istream &in);
    static srs_mpc_phase2_accumulator<ppT> read_compressed(std::istream &in);
};

/// Public Key representing a single contribution to the MPC. This includes a
/// proof-of-knowledge, consisting of some point $s$ in G1, chosen randomly, $s
/// * \delta$, $r$ generated deterministically by hashing $s$, $s * \delta$ and
/// the current transcript digest into G2, and $r * \delta$.
///
/// Similarly to the "Phase2" library (https://github.com/ebfull/phase2), we
/// track both the new value of delta (after applying this contribution),
/// and the transaction digest before applying, in order that we can
/// efficiently check the transcript using just the public keys.
///
/// Implements the interfaces of StructuredT and ReadableT templates.
template<typename ppT> class srs_mpc_phase2_publickey
{
public:
    mpc_hash_t transcript_digest;
    libff::G1<ppT> new_delta_g1;
    libff::G1<ppT> s_g1;
    libff::G1<ppT> s_delta_j_g1;
    libff::G2<ppT> r_delta_j_g2;

    srs_mpc_phase2_publickey(
        const mpc_hash_t transcript_digest,
        const libff::G1<ppT> &new_delta_g1,
        const libff::G1<ppT> &s_g1,
        const libff::G1<ppT> &s_delta_j_g1,
        const libff::G2<ppT> &r_delta_j_g2);

    bool operator==(const srs_mpc_phase2_publickey<ppT> &other) const;
    bool is_well_formed() const;
    void write(std::ostream &out) const;
    static srs_mpc_phase2_publickey<ppT> read(std::istream &in);
    void compute_digest(mpc_hash_t out_digest) const;
};

/// Challenge given to a participant in Phase2 of the SRS generation MPC.
/// Implements the interfaces of StructuredT and ReadableT templates.
template<typename ppT> class srs_mpc_phase2_challenge
{
public:
    mpc_hash_t transcript_digest;
    srs_mpc_phase2_accumulator<ppT> accumulator;

    srs_mpc_phase2_challenge(
        const mpc_hash_t transcript_digest,
        srs_mpc_phase2_accumulator<ppT> &&accumulator);

    bool operator==(const srs_mpc_phase2_challenge<ppT> &other) const;
    bool is_well_formed() const;
    void write(std::ostream &out) const;
    static srs_mpc_phase2_challenge<ppT> read(std::istream &in);
};

/// Reponse produced by participant in Phase2 of the SRS generation MPC.
/// Implements the interfaces of StructuredT and ReadableT templates.
template<typename ppT> class srs_mpc_phase2_response
{
public:
    srs_mpc_phase2_accumulator<ppT> new_accumulator;
    srs_mpc_phase2_publickey<ppT> publickey;

    srs_mpc_phase2_response(
        srs_mpc_phase2_accumulator<ppT> &&new_accumulator,
        srs_mpc_phase2_publickey<ppT> &&publickey);

    bool operator==(const srs_mpc_phase2_response<ppT> &other) const;
    bool is_well_formed() const;
    void write(std::ostream &out) const;
    static srs_mpc_phase2_response<ppT> read(std::istream &in);
};

// Phase2 functions

template<mp_size_t n, const libff::bigint<n> &modulus>
void srs_mpc_digest_to_fp(
    const mpc_hash_t transcript_digest, libff::Fp_model<n, modulus> &out_fr);

template<typename ppT>
libff::G2<ppT> srs_mpc_digest_to_g2(const mpc_hash_t digest);

/// Given the output from the linear combination of the L1 layer of the SRS
/// circuit, compute the starting parameters for Phase 2 (the MPC for C2
/// layer). See "Initialization" in section 7.3 of [BoweGM17].
template<typename ppT>
srs_mpc_phase2_accumulator<ppT> srs_mpc_phase2_begin(
    const mpc_hash_t cs_hash,
    const srs_mpc_layer_L1<ppT> &layer_L1,
    size_t num_inputs);

/// Outputs the public key (which includes the POK) for our secret. Correponds
/// to steps 1 and 2 in "Computation", section 7.3 of [BoweGM17]
template<typename ppT>
srs_mpc_phase2_publickey<ppT> srs_mpc_phase2_compute_public_key(
    const mpc_hash_t transcript_digest,
    const libff::G1<ppT> &last_delta,
    const libff::Fr<ppT> &secret);

/// Verifies that a public key is correct, given some previous delta.
/// Corresponds to steps 1 and 2 from "Verification" in section 7.3 of
/// [BoweGM17] (for a single contributor $j$). Note that the caller is
/// responsible for checking the transcript_hash.
template<typename ppT>
bool srs_mpc_phase2_verify_publickey(
    const libff::G1<ppT> last_delta_g1,
    const srs_mpc_phase2_publickey<ppT> &publickey);

/// Core update function, which applies a secret contribution to an
/// accumulator. Corresponds to steps 3 onwards in "Computation", section 7.3
/// of [BoweGM17].
template<typename ppT>
srs_mpc_phase2_accumulator<ppT> srs_mpc_phase2_update_accumulator(
    const srs_mpc_phase2_accumulator<ppT> &last_accum,
    const libff::Fr<ppT> &delta_j);

/// Assuming last is fully verified, and updated.delta_g1 has the appropriate
/// ratio, check that all other elements of updated are correct. This covers
/// the G2 part of step 2, and all of steps 3 and 4 of "Verification" in
/// section 7.3 of [BoweGM17]. Primarily used directly by
/// `srs_mpc_phase2_verify_update`, as part of the validation process for a
/// contribution and resulting accumulator. It also used when verifying the
/// final transcript, to check consistency of initial and final accumulators
/// without intermediate values.
template<typename ppT>
bool srs_mpc_phase2_update_is_consistent(
    const srs_mpc_phase2_accumulator<ppT> &last,
    const srs_mpc_phase2_accumulator<ppT> &updated);

/// Core verification function for a single contribution. Checks the
/// self-consistency of a public key, and that the corresponding contribution
/// has been correctly applied to all values in 'last', to generate 'updated'.
/// This corresponds to "Verification" in section 7.3 of [BoweGM17] (for a
/// single contributor $j$). Note that the caller must verify that
/// publickey.transcript_digest corresponds the correct challenge.
template<typename ppT>
bool srs_mpc_phase2_verify_update(
    const srs_mpc_phase2_accumulator<ppT> &last,
    const srs_mpc_phase2_accumulator<ppT> &updated,
    const srs_mpc_phase2_publickey<ppT> &publickey);

/// Given an initial accumulator, create the first challenge object. Uses the
/// hash of the empty string for the initial transcript digest.
template<typename ppT>
srs_mpc_phase2_challenge<ppT> srs_mpc_phase2_initial_challenge(
    srs_mpc_phase2_accumulator<ppT> &&initial_accumulator);

/// Wraps `srs_mpc_phase2_compute_public_key` and
/// `srs_mpc_phase2_update_accumulator` to implement the "Computation"
/// operation in section 7.3 of [BoweGM17], and produce a response for the
/// protocol coordinator.
template<typename ppT>
srs_mpc_phase2_response<ppT> srs_mpc_phase2_compute_response(
    const srs_mpc_phase2_challenge<ppT> &challenge,
    const libff::Fr<ppT> &delta_j);

/// Verify a response against a given challenge. Checks that the response
/// matches the expected hash in the challenge, and leverages
/// `srs_mpc_phase2_verify_update` to validate the claimed contribution.
template<typename ppT>
bool srs_mpc_phase2_verify_response(
    const srs_mpc_phase2_challenge<ppT> &challenge,
    const srs_mpc_phase2_response<ppT> &response);

/// Given a `response` (which should already have been validated with
/// `srs_mpc_phase2_verify_response`), create a new challenge object. This
/// essentially copies the accumulator, and updates the transcript digest for
/// the next contributor.
template<typename ppT>
srs_mpc_phase2_challenge<ppT> srs_mpc_phase2_compute_challenge(
    srs_mpc_phase2_response<ppT> &&response);

/// The transcript of the MPC is formed of a sequence of contributions (public
/// key). Each contribution contains the digest of the previous one, and the
/// value of delta after it has been applied. Therefore the final accumulator
/// can be fully verified using just the transcript as follows:
///
/// - establish the initial value of accumulator and the transcript digest
///
/// - confirm the transcript of contributions based on this, yielding an
///   encoding of the final delta
///
/// - check that the final accumulator is consistent with the initial
///   accumulator, based on the final delta
///
/// This function validates the transcript as a stream of publickey objects,
/// outputing the encoding of the final delta in G1.
template<typename ppT, bool enable_contribution_check = true>
bool srs_mpc_phase2_verify_transcript(
    const mpc_hash_t initial_transcript_digest,
    const libff::G1<ppT> &initial_delta,
    const mpc_hash_t check_for_contribution,
    std::istream &transcript_stream,
    libff::G1<ppT> &out_final_delta,
    mpc_hash_t out_final_transcript_digest,
    bool &out_contribution_found);

/// Similar to other transcript verification above, but does not check for the
/// presence of a specific contribution digest.
template<typename ppT>
bool srs_mpc_phase2_verify_transcript(
    const mpc_hash_t initial_transcript_digest,
    const libff::G1<ppT> &initial_delta,
    std::istream &transcript_stream,
    libff::G1<ppT> &out_final_delta,
    mpc_hash_t out_final_transcript_digest);

/// Given the output from the first layer of the MPC, perform the 2nd
/// layer computation using just local randomness for delta. This is not a
/// substitute for the full MPC with an auditable log of
/// contributions, but is useful for testing.
template<typename ppT>
srs_mpc_phase2_challenge<ppT> srs_mpc_dummy_phase2(
    const srs_mpc_layer_L1<ppT> &layer1,
    const libff::Fr<ppT> &delta,
    size_t num_inputs);

/// Given the output from all phases of the MPC, create the proving and
/// verification keys for the given circuit.
template<typename ppT>
libsnark::r1cs_gg_ppzksnark_keypair<ppT> mpc_create_key_pair(
    srs_powersoftau<ppT> &&pot,
    srs_mpc_layer_L1<ppT> &&layer1,
    srs_mpc_phase2_accumulator<ppT> &&layer2,
    libsnark::r1cs_constraint_system<libff::Fr<ppT>> &&cs,
    const libsnark::qap_instance<libff::Fr<ppT>> &qap);

} // namespace libzeth

#include "libzeth/mpc/groth16/phase2.tcc"

#endif // __ZETH_MPC_GROTH16_PHASE2_HPP__
