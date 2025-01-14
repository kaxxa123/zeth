// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "libzeth/core/field_element_utils.hpp"
#include "libzeth/core/group_element_utils.hpp"
#include "libzeth/serialization/proto_utils.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/algebra/curves/bls12_377/bls12_377_pp.hpp>
#include <libff/algebra/curves/bw6_761/bw6_761_pp.hpp>
#include <libff/algebra/curves/curve_utils.hpp>

using namespace libzeth;

boost::filesystem::path g_output_dir = boost::filesystem::path("");

namespace
{

template<typename ppT> void operation_test_data()
{
    // Generate data to be used to check the following operations
    //
    //   ecadd:
    //       [6] == [2] + [4]   (in G1)
    //   ecmul:
    //       [-8] == -2 * [4]   (in G1)
    //   ecpairing:
    //       e([6], [4]) * e([3],[8]) * e([4],[4]) * e([-8], [8]) == 1
    //
    // where [] represents exponentiation (all elements are in the
    // appropriate groups and fields, defined by context).

    const libff::Fr<ppT> fr_1 = libff::Fr<ppT>::one();
    const libff::Fr<ppT> fr_2 = fr_1 + fr_1;
    // const libff::Fr<ppT> fr_3 = fr_2 + fr_1;
    const libff::Fr<ppT> fr_minus_2 = -fr_2;

    const libff::G1<ppT> g1_1 = libff::G1<ppT>::one();
    const libff::G1<ppT> g1_2 = fr_2 * g1_1;
    const libff::G1<ppT> g1_3 = g1_1 + g1_2;
    const libff::G1<ppT> g1_4 = g1_2 + g1_2;
    const libff::G1<ppT> g1_6 = g1_2 + g1_4;
    const libff::G1<ppT> g1_8 = g1_4 + g1_4;

    const libff::G2<ppT> g2_1 = libff::G2<ppT>::one();
    const libff::G2<ppT> g2_2 = fr_2 * g2_1;
    const libff::G2<ppT> g2_4 = g2_2 + g2_2;
    const libff::G2<ppT> g2_8 = g2_4 + g2_4;

    // Simultaneously write all field and group  data to a string stream.
    std::stringstream binary_stream;

    std::cout << " Fr:";
    std::cout << "\n    r: "
              << bigint_to_hex<libff::Fr<ppT>>(libff::Fr<ppT>::mod);

    std::cout << "\n   1: ";
    field_element_write_json(fr_1, std::cout);
    field_element_write_bytes(fr_1, binary_stream);
    std::cout << "\n   2: ";
    field_element_write_json(fr_2, std::cout);
    field_element_write_bytes(fr_2, binary_stream);
    std::cout << "\n   -2: ";
    field_element_write_json(fr_minus_2, std::cout);
    field_element_write_bytes(fr_minus_2, binary_stream);

    std::cout << " Fq:";
    std::cout << "\n    q: "
              << bigint_to_hex<libff::Fq<ppT>>(libff::Fq<ppT>::mod);

    std::cout << "\n G1:";
    std::cout << "\n   1: ";
    group_element_write_json(g1_1, std::cout);
    group_element_write_bytes(g1_1, binary_stream);
    std::cout << "\n  -1: ";
    group_element_write_json(-g1_1, std::cout);
    group_element_write_bytes(-g1_1, binary_stream);
    std::cout << "\n   2: ";
    group_element_write_json(g1_2, std::cout);
    group_element_write_bytes(g1_2, binary_stream);
    std::cout << "\n   3: ";
    group_element_write_json(g1_3, std::cout);
    group_element_write_bytes(g1_3, binary_stream);
    std::cout << "\n   4: ";
    group_element_write_json(g1_4, std::cout);
    group_element_write_bytes(g1_4, binary_stream);
    std::cout << "\n   6: ";
    group_element_write_json(g1_6, std::cout);
    group_element_write_bytes(g1_6, binary_stream);
    std::cout << "\n   8: ";
    group_element_write_json(g1_8, std::cout);
    group_element_write_bytes(g1_8, binary_stream);
    std::cout << "\n  -8: ";
    group_element_write_json(-g1_8, std::cout);
    group_element_write_bytes(-g1_8, binary_stream);

    std::cout << "\n G2:";
    std::cout << "\n   1: ";
    group_element_write_json(g2_1, std::cout);
    group_element_write_bytes(g2_1, binary_stream);
    std::cout << "\n  -1: ";
    group_element_write_json(-g2_1, std::cout);
    group_element_write_bytes(-g2_1, binary_stream);
    std::cout << "\n   4: ";
    group_element_write_json(g2_4, std::cout);
    group_element_write_bytes(g2_4, binary_stream);
    std::cout << "\n   8: ";
    group_element_write_json(g2_8, std::cout);
    group_element_write_bytes(g2_8, binary_stream);
    std::cout << "\n  -8: ";
    group_element_write_json(-g2_8, std::cout);
    group_element_write_bytes(-g2_8, binary_stream);
    std::cout << "\n";

    // Write the binary data if output_dir given
    if (!g_output_dir.empty()) {
        boost::filesystem::path outpath =
            g_output_dir / ("ec_test_data_" + libzeth::pp_name<ppT>() + ".bin");
        std::cout << "(writing to file " << outpath << ")\n";
        std::ofstream out_s(
            outpath.c_str(), std::ios_base::out | std::ios_base::binary);
        out_s << binary_stream.str();
    } else {
        std::cout << "(skipping binary data write)\n";
    }

    // Check the statements above
    ASSERT_EQ(g1_6, g1_2 + g1_4);
    ASSERT_EQ(-g1_8, fr_minus_2 * g1_4);
    ASSERT_EQ(
        ppT::reduced_pairing(g1_6, g2_4) * ppT::reduced_pairing(g1_3, g2_8) *
            ppT::reduced_pairing(g1_4, g2_4) *
            ppT::reduced_pairing(-g1_8, g2_8),
        libff::GT<ppT>::one());
}

TEST(ECOperationDataTest, ALT_BN128)
{
    std::cout << "ALT_BN128:\n";
    operation_test_data<libff::alt_bn128_pp>();
}

TEST(ECOperationDataTest, BW6_761)
{
    using G1 = libff::bw6_761_G1;
    using G2 = libff::bw6_761_G2;
    using Fq = typename G1::base_field;
    using Fqe = typename G2::twist_field;

    std::cout << "BW6-761:\n";
    operation_test_data<libff::bw6_761_pp>();

    // Print some invalid points for use in tests that require them. Points
    // outside safe subgroup are from the libff unit tests
    // (libff/algebra/curves/tests/test_groups.cpp)

    const G1 g1_not_well_formed(Fq::one(), Fq::one(), Fq::one());
    ASSERT_FALSE(g1_not_well_formed.is_well_formed());

    const G1 g1_not_in_subgroup = libff::g1_curve_point_at_x<G1>(Fq("6"));
    ASSERT_TRUE(g1_not_in_subgroup.is_well_formed());
    ASSERT_FALSE(g1_not_in_subgroup.is_in_safe_subgroup());

    const G2 g2_not_well_formed(Fqe::one(), Fqe::one(), Fqe::one());
    ASSERT_FALSE(g2_not_well_formed.is_well_formed());

    const G2 g2_not_in_subgroup = libff::g2_curve_point_at_x<G2>(Fqe::zero());
    ASSERT_TRUE(g2_not_in_subgroup.is_well_formed());
    ASSERT_FALSE(g2_not_in_subgroup.is_in_safe_subgroup());

    std::cout << "   g1_not_well_formed: ";
    group_element_write_json(g1_not_well_formed, std::cout);
    std::cout << "\n   g1_not_in_subgroup: ";
    group_element_write_json(g1_not_in_subgroup, std::cout);
    std::cout << "\n   g2_not_well_formed: ";
    group_element_write_json(g2_not_well_formed, std::cout);
    std::cout << "\n   g2_not_in_subgroup: ";
    group_element_write_json(g2_not_in_subgroup, std::cout);
    std::cout << "\n";
}

TEST(ECOperationDataTest, BLS12_377)
{
    using G1 = libff::bls12_377_G1;
    using G2 = libff::bls12_377_G2;
    using Fq = typename G1::base_field;
    using Fqe = typename G2::twist_field;

    std::cout << "BLS12-377:\n";
    operation_test_data<libff::bls12_377_pp>();

    // Print some invalid points for use in tests that require them. Points
    // outside safe subgroup are from the libff unit tests
    // (libff/algebra/curves/tests/test_groups.cpp)

    const G1 g1_not_well_formed(Fq::one(), Fq::one(), Fq::one());
    ASSERT_FALSE(g1_not_well_formed.is_well_formed());

    const G1 g1_not_in_subgroup = libff::g1_curve_point_at_x<G1>(Fq("3"));
    ASSERT_TRUE(g1_not_in_subgroup.is_well_formed());
    ASSERT_FALSE(g1_not_in_subgroup.is_in_safe_subgroup());

    const G2 g2_not_well_formed(Fqe::one(), Fqe::one(), Fqe::one());
    ASSERT_FALSE(g2_not_well_formed.is_well_formed());

    const G2 g2_not_in_subgroup =
        libff::g2_curve_point_at_x<G2>(Fq(3) * Fqe::one());
    ASSERT_TRUE(g2_not_in_subgroup.is_well_formed());
    ASSERT_FALSE(g2_not_in_subgroup.is_in_safe_subgroup());

    std::cout << "   g1_not_well_formed: ";
    group_element_write_json(g1_not_well_formed, std::cout);
    std::cout << "\n   g1_not_in_subgroup: ";
    group_element_write_json(g1_not_in_subgroup, std::cout);
    std::cout << "\n   g2_not_well_formed: ";
    group_element_write_json(g2_not_well_formed, std::cout);
    std::cout << "\n   g2_not_in_subgroup: ";
    group_element_write_json(g2_not_in_subgroup, std::cout);
    std::cout << "\n";
}

} // namespace

int main(int argc, char **argv)
{
    libff::alt_bn128_pp::init_public_params();
    libff::bls12_377_pp::init_public_params();
    libff::bw6_761_pp::init_public_params();
    ::testing::InitGoogleTest(&argc, argv);

    // Extract the test data destination dir, if passed on the command line.
    if (argc > 1) {
        g_output_dir = boost::filesystem::path(argv[1]);
    }

    return RUN_ALL_TESTS();
}
