// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "libzeth/core/group_element_utils.hpp"

#include <gtest/gtest.h>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/algebra/curves/bls12_377/bls12_377_pp.hpp>
#include <libff/algebra/curves/bw6_761/bw6_761_pp.hpp>
#include <libff/algebra/curves/mnt/mnt4/mnt4_pp.hpp>
#include <libff/algebra/curves/mnt/mnt6/mnt6_pp.hpp>

namespace
{

TEST(GroupElementUtilsTest, G1EncodeDecodeJsonTestVectorAltBN128)
{
    using pp = libff::alt_bn128_pp;
    using Fr = libff::Fr<pp>;
    using G1 = libff::G1<pp>;

    static const std::string g1_json_expected =
        "["
        "\"0x05e86f8cc8a7a4f10f56093465679f17f8b8c3fdb41469e408b529e030f52f3f\""
        ","
        "\"0x2857bd14bbc09767bed8e913d3ccb42b2bc8738f715417dd6f020725d22bcd90\""
        "]";
    G1 g1 = Fr(13) * G1::one();
    std::string g1_json = libzeth::group_element_to_json(g1);
    ASSERT_EQ(g1_json_expected, g1_json);

    G1 g1_decoded = libzeth::group_element_from_json<G1>(g1_json);
    ASSERT_EQ(g1, g1_decoded);
}

TEST(GroupElementUtilsTest, G2EncodeJsonTestVectorAltBN128)
{
    using pp = libff::alt_bn128_pp;
    using G2 = libff::G2<pp>;

    static const std::string g2_json_expected =
        "[["
        "\"0x198e9393920d483a7260bfb731fb5d25f1aa493335a9e71297e485b7aef312c2\""
        ","
        "\"0x1800deef121f1e76426a00665e5c4479674322d4f75edadd46debd5cd992f6ed\""
        "],["
        "\"0x090689d0585ff075ec9e99ad690c3395bc4b313370b38ef355acdadcd122975b\""
        ","
        "\"0x12c85ea5db8c6deb4aab71808dcb408fe3d1e7690c43d37b4ce6cc0166fa7daa\""
        "]]";
    G2 g2 = G2::one();
    const std::string g2_json = libzeth::group_element_to_json(g2);
    const G2 g2_decoded = libzeth::group_element_from_json<G2>(g2_json);

    ASSERT_EQ(g2_json_expected, g2_json);
    ASSERT_EQ(g2, g2_decoded);
}

template<typename GroupT>
static void single_group_element_encode_decode_json_test(const GroupT &g)
{
    const std::string g_json = libzeth::group_element_to_json(g);
    const GroupT g_decoded = libzeth::group_element_from_json<GroupT>(g_json);

    ASSERT_EQ(g, g_decoded);
}

template<typename GroupT> static void group_element_encode_decode_json_test()
{
    single_group_element_encode_decode_json_test(GroupT::random_element());
    single_group_element_encode_decode_json_test(GroupT::zero());
    single_group_element_encode_decode_json_test(GroupT::one());
}

TEST(GroupElementUtilsTest, G1EncodeDecodeJSON)
{
    group_element_encode_decode_json_test<libff::G1<libff::alt_bn128_pp>>();
    group_element_encode_decode_json_test<libff::G1<libff::mnt4_pp>>();
    group_element_encode_decode_json_test<libff::G1<libff::mnt6_pp>>();
    group_element_encode_decode_json_test<libff::G1<libff::bls12_377_pp>>();
    group_element_encode_decode_json_test<libff::G1<libff::bw6_761_pp>>();
}

TEST(GroupElementUtilsTest, G2EncodeDecodeJSON)
{
    group_element_encode_decode_json_test<libff::G2<libff::alt_bn128_pp>>();
    group_element_encode_decode_json_test<libff::G2<libff::mnt4_pp>>();
    group_element_encode_decode_json_test<libff::G2<libff::mnt6_pp>>();
    group_element_encode_decode_json_test<libff::G2<libff::bls12_377_pp>>();
    group_element_encode_decode_json_test<libff::G2<libff::bw6_761_pp>>();
}

template<typename GroupT>
static void single_group_element_encode_decode_bytes_test(const GroupT &g)
{
    std::string buffer;
    {
        std::stringstream ss;
        libzeth::group_element_write_bytes(g, ss);
        buffer = ss.str();
    }

    GroupT g_decoded;
    {
        std::stringstream ss(buffer);
        libzeth::group_element_read_bytes(g_decoded, ss);
    }

    ASSERT_EQ(g, g_decoded);
}

template<typename GroupT> static void group_element_encode_decode_bytes_test()
{
    single_group_element_encode_decode_bytes_test(GroupT::random_element());
    single_group_element_encode_decode_bytes_test(GroupT::zero());
    single_group_element_encode_decode_bytes_test(GroupT::one());
}

TEST(GroupElementUtilsTest, G1EncodeDecodeBytes)
{
    group_element_encode_decode_bytes_test<libff::G1<libff::alt_bn128_pp>>();
    group_element_encode_decode_bytes_test<libff::G1<libff::mnt4_pp>>();
    group_element_encode_decode_bytes_test<libff::G1<libff::mnt6_pp>>();
    group_element_encode_decode_bytes_test<libff::G1<libff::bls12_377_pp>>();
    group_element_encode_decode_bytes_test<libff::G1<libff::bw6_761_pp>>();
}

TEST(GroupElementUtilsTest, G2EncodeDecodeBytes)
{
    group_element_encode_decode_bytes_test<libff::G2<libff::alt_bn128_pp>>();
    group_element_encode_decode_bytes_test<libff::G2<libff::mnt4_pp>>();
    group_element_encode_decode_bytes_test<libff::G2<libff::mnt6_pp>>();
    group_element_encode_decode_bytes_test<libff::G2<libff::bls12_377_pp>>();
    group_element_encode_decode_bytes_test<libff::G2<libff::bw6_761_pp>>();
}

template<typename GroupT> static void group_elements_encode_decode_bytes_test()
{
    const size_t num_elements = 17;
    std::vector<GroupT> elements;
    elements.reserve(num_elements);
    for (size_t i = 0; i < num_elements; ++i) {
        elements.emplace_back(GroupT::random_element());
    }

    std::string buffer;
    {
        std::stringstream ss;
        libzeth::group_elements_write_bytes(elements, ss);
        buffer = ss.str();
    }

    std::vector<GroupT> elements_decoded;
    {
        std::stringstream ss(buffer);
        libzeth::group_elements_read_bytes(elements_decoded, ss);
    }

    ASSERT_EQ(elements, elements_decoded);
}

TEST(GroupElementUtilsTest, G1CollectionEncodeDecodeBytes)
{
    group_elements_encode_decode_bytes_test<libff::G1<libff::alt_bn128_pp>>();
    group_elements_encode_decode_bytes_test<libff::G1<libff::mnt4_pp>>();
    group_elements_encode_decode_bytes_test<libff::G1<libff::mnt6_pp>>();
    group_elements_encode_decode_bytes_test<libff::G1<libff::bls12_377_pp>>();
    group_elements_encode_decode_bytes_test<libff::G1<libff::bw6_761_pp>>();
}

TEST(GroupElementUtilsTest, G2CollectionEncodeDecodeBytes)
{
    group_elements_encode_decode_bytes_test<libff::G2<libff::alt_bn128_pp>>();
    group_elements_encode_decode_bytes_test<libff::G2<libff::mnt4_pp>>();
    group_elements_encode_decode_bytes_test<libff::G2<libff::mnt6_pp>>();
    group_elements_encode_decode_bytes_test<libff::G2<libff::bls12_377_pp>>();
    group_elements_encode_decode_bytes_test<libff::G2<libff::bw6_761_pp>>();
}

} // namespace

int main(int argc, char **argv)
{
    libff::alt_bn128_pp::init_public_params();
    libff::mnt4_pp::init_public_params();
    libff::mnt6_pp::init_public_params();
    libff::bls12_377_pp::init_public_params();
    libff::bw6_761_pp::init_public_params();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
