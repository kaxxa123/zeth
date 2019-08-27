#include "snarks/groth16/mpc_utils.hpp"
#include "snarks/groth16/multi_exp.hpp"
#include "test/simple_test.hpp"
#include "util.hpp"

#include <gtest/gtest.h>

using ppT = libff::default_ec_pp;
using Fr = libff::Fr<ppT>;
using G1 = libff::G1<ppT>;
using G2 = libff::G2<ppT>;
using namespace libsnark;
using namespace libzeth;

namespace
{

TEST(MPCTests, LinearCombination)
{
    // Compute the small test qap first, in order to extract the
    // degree.
    const r1cs_constraint_system<Fr> constraint_system = ([] {
        protoboard<Fr> pb;
        libzeth::test::simple_circuit<Fr>(pb);
        r1cs_constraint_system<Fr> cs = pb.get_constraint_system();
        cs.swap_AB_if_beneficial();
        return cs;
    })();
    qap_instance<Fr> qap = r1cs_to_qap_instance_map(constraint_system);

    // dummy powersoftau
    Fr tau = Fr::random_element();
    Fr alpha = Fr::random_element();
    Fr beta = Fr::random_element();
    const srs_powersoftau pot =
        dummy_powersoftau_from_secrets(tau, alpha, beta, qap.degree());

    // linear combination
    const srs_mpc_layer_L1 layer1 = mpc_compute_linearcombination(pot, qap);

    // Without knowlege of tau, not many checks can be performed
    // beyond the ratio of terms in [ t(x) . x^i ]_1.
    const size_t qap_n = qap.degree();
    ASSERT_EQ(qap_n - 1, layer1.T_tau_powers_g1.size());
    ASSERT_EQ(qap.num_variables() + 1, layer1.ABC_g1.size());

    for (size_t i = 1; i < qap_n - 1; ++i) {
        ASSERT_TRUE(::same_ratio<ppT>(
            layer1.T_tau_powers_g1[i - 1],
            layer1.T_tau_powers_g1[i],
            pot.tau_powers_g2[0],
            pot.tau_powers_g2[1]))
            << "i = " << std::to_string(i);
    }

    // Use knowledge of secrets to confirm values.
    // Check that:
    //
    //   [ domain.Z(tau) ]_1 = layer1.T_tau_powers_g1[0]
    //   [ beta . A_i(tau) + alpha . B_i(tau) + C_i(tau) ]_1 = layer1.ABC_g1[i]
    {
        const qap_instance_evaluation<Fr> qap_evaluation = ([&tau] {
            protoboard<Fr> pb;
            libzeth::test::simple_circuit<Fr>(pb);
            r1cs_constraint_system<Fr> constraint_system =
                pb.get_constraint_system();
            constraint_system.swap_AB_if_beneficial();
            return r1cs_to_qap_instance_map_with_evaluation(
                constraint_system, tau);
        })();

        ASSERT_EQ(
            qap_evaluation.domain->compute_vanishing_polynomial(tau) *
                G1::one(),
            layer1.T_tau_powers_g1[0]);

        for (size_t i = 0; i < qap_evaluation.num_variables() + 1; ++i) {
            // At
            ASSERT_EQ(qap_evaluation.At[i] * G1::one(), layer1.A_g1[i]);

            // Bt
            ASSERT_EQ(qap_evaluation.Bt[i] * G1::one(), layer1.B_g1[i]);
            ASSERT_EQ(qap_evaluation.Bt[i] * G2::one(), layer1.B_g2[i]);

            // ABCt
            const Fr ABC_i = beta * qap_evaluation.At[i] +
                             alpha * qap_evaluation.Bt[i] +
                             qap_evaluation.Ct[i];
            ASSERT_EQ(ABC_i * G1::one(), layer1.ABC_g1[i]);
        }
    }
}

} // namespace

int main(int argc, char **argv)
{
    // !!! WARNING: Do not forget to do this once for all tests !!!
    ppT::init_public_params();

    // Remove stdout noise from libff
    libff::inhibit_profiling_counters = true;
    libff::inhibit_profiling_info = true;

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}