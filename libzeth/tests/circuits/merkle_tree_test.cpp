// Copyright (c) 2015-2021 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "libzeth/circuits/merkle_tree/merkle_path_authenticator.hpp"
#include "libzeth/circuits/merkle_tree/merkle_path_selector.hpp"
#include "libzeth/circuits/mimc/mimc_mp.hpp"

#include <gtest/gtest.h>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>

using namespace libzeth;

template<typename FieldT>
using MiMCe7_permutation_gadget = MiMC_permutation_gadget<FieldT, 17, 65>;

// Instantiation of the templates for the tests.  Data here assumes alt_bn128
using pp = libff::alt_bn128_pp;
using Field = libff::Fr<pp>;
using HashTree = MiMC_mp_gadget<Field, MiMCe7_permutation_gadget<Field>>;

namespace
{

bool test_merkle_path_selector(int is_right)
{
    libsnark::protoboard<Field> pb;

    Field value_A = Field("14967453892511805220505707596666005495248157115618"
                          "6698930522557832224430770");
    Field value_B = Field("96707014654643119032492206924834019388884986418749"
                          "48577387207195814981706974");

    is_right = is_right ? 1 : 0;

    libsnark::pb_variable<Field> var_A;
    var_A.allocate(pb, "var_A");

    pb.val(var_A) = value_A;

    libsnark::pb_variable<Field> var_B;
    var_B.allocate(pb, "var_B");
    pb.val(var_B) = value_B;

    libsnark::pb_variable<Field> var_is_right;
    var_is_right.allocate(pb, "var_is_right");
    pb.val(var_is_right) = is_right;

    merkle_path_selector<Field> selector(
        pb, var_A, var_B, var_is_right, "test_merkle_path_selector");

    selector.generate_r1cs_constraints();
    selector.generate_r1cs_witness();

    if (is_right) {
        if ((pb.val(selector.get_left()) != value_B) &&
            (pb.val(selector.get_right()) != value_A)) {
            return false;
        }
    } else {
        if ((pb.val(selector.get_left()) != value_A) &&
            (pb.val(selector.get_right()) != value_B)) {
            return false;
        }
    }

    if (!pb.is_satisfied()) {
        std::cerr << "FAIL merkle_path_authenticator is_satisfied" << std::endl;
        return false;
    }

    return true;
}

bool test_merkle_path_authenticator_depth1()
{
    libsnark::protoboard<Field> pb;

    // Tree depth is 1 for this test
    size_t tree_depth = 1;

    // left leaf:
    // 3703141493535563179657531719960160174296085208671919316200479060314459804651,
    // right leaf:
    // 134551314051432487569247388144051420116740427803855572138106146683954151557,
    // root:
    // hash(left, right)
    Field left = Field("37031414935355631796575317199601601742960852086719193"
                       "16200479060314459804651");
    Field right = Field("1345513140514324875692473881440514201167404278038555"
                        "72138106146683954151557");
    Field root = HashTree::get_hash(left, right);

    // Set the authenticator for right leaf (`is_right` = 1)
    Field is_right = 1;

    libsnark::pb_variable<Field> expected_root;
    expected_root.allocate(pb, "expected_root");
    pb.val(expected_root) = root;
    pb.set_input_sizes(1);

    // Bit representation of the address of the leaf to authenticate (here: 1)
    //
    // Note: In a tree of depth d, there are 2^d leaves
    // Each of them can, then, be given an address/index encoded on d bits.
    libsnark::pb_variable_array<Field> address_bits;
    address_bits.allocate(pb, tree_depth, "address_bits");
    pb.val(address_bits[0]) = is_right;

    libsnark::pb_variable_array<Field> path;
    path.allocate(pb, 1, "path");
    pb.val(path[0]) = left;

    libsnark::pb_variable<Field> leaf;
    leaf.allocate(pb, "leaf");
    pb.val(leaf) = right;

    libsnark::pb_variable<Field> enforce_bit;
    enforce_bit.allocate(pb, "enforce_bit");
    pb.val(enforce_bit) = Field("1");

    merkle_path_authenticator<Field, HashTree> auth(
        pb,
        tree_depth,
        address_bits,
        leaf,
        expected_root,
        path,
        enforce_bit,
        "authenticator");

    auth.generate_r1cs_constraints();
    auth.generate_r1cs_witness();

    if (!auth.is_valid()) {
        std::cerr << "Not valid!" << std::endl;
        std::cerr << "Expected ";
        pb.val(expected_root).print();
        std::cerr << "Actual ";
        pb.val(auth.result()).print();
        return false;
    }

    if (!pb.is_satisfied()) {
        std::cerr << "Constraint system not satisfied!" << std::endl;
        return false;
    }

    return true;
}

bool test_merkle_path_authenticator_depth3()
{
    libsnark::protoboard<Field> pb;

    // Tree depth is 3 for this test
    size_t tree_depth = 3;

    // We want to authenticate `right0`
    // Thus, we want to check that hash(left2, hash(left1, hash(left0, right0)))
    // == root Where leftX (resp. rightX) denotes the left (resp. right) leaf at
    // level X in the tree (starting from the leaf level being 0)
    Field left0 = Field("0");
    Field right0 = Field("0");
    Field left1 = Field("1171400889311693944151078859955763681651852732754319"
                        "3374630310875272509334396");
    Field left2 = Field("9881790034808292405036271961589462686158587796044671"
                        "417688221824074647491645");
    Field root = HashTree::get_hash(
        left2, HashTree::get_hash(left1, HashTree::get_hash(left0, right0)));
    Field is_right = 1;

    // Bit representation of the leaf to authenticate
    // (here: (111)_2 = (7)_10) (_X denote encoding in base X)
    libsnark::pb_variable_array<Field> address_bits;
    address_bits.allocate(pb, tree_depth, "address_bits");
    pb.val(address_bits[0]) = is_right;
    pb.val(address_bits[1]) = is_right;
    pb.val(address_bits[2]) = is_right;

    libsnark::pb_variable_array<Field> path;
    path.allocate(pb, 3, "path");
    pb.val(path[0]) = left0;
    pb.val(path[1]) = left1;
    pb.val(path[2]) = left2;

    libsnark::pb_variable<Field> leaf;
    leaf.allocate(pb, "leaf");
    pb.val(leaf) = right0;

    libsnark::pb_variable<Field> expected_root;
    expected_root.allocate(pb, "expected_root");
    pb.val(expected_root) = root;

    libsnark::pb_variable<Field> enforce_bit;
    enforce_bit.allocate(pb, "enforce_bit");
    pb.val(enforce_bit) = Field("1");

    merkle_path_authenticator<Field, HashTree> auth(
        pb,
        tree_depth,
        address_bits,
        leaf,
        expected_root,
        path,
        enforce_bit,
        "authenticator");

    auth.generate_r1cs_constraints();
    auth.generate_r1cs_witness();

    if (!auth.is_valid()) {
        std::cerr << "Not valid!" << std::endl;
        std::cerr << "Expected ";
        pb.val(expected_root).print();
        std::cerr << "Actual ";
        pb.val(auth.result()).print();
        return false;
    }

    if (!pb.is_satisfied()) {
        std::cerr << "Not satisfied!" << std::endl;
        return false;
    }

    return true;
}

TEST(MerkleTreeTest, PathSelector0)
{
    ASSERT_TRUE(test_merkle_path_selector(0));
}

TEST(MerkleTreeTest, PathSelector1)
{
    ASSERT_TRUE(test_merkle_path_selector(1));
}

TEST(MerkleTreeTest, PathAuthenticatorDepth1)
{
    ASSERT_TRUE(test_merkle_path_authenticator_depth1());
}

TEST(MerkleTreeTest, PathAuthenticatorDepth3)
{
    ASSERT_TRUE(test_merkle_path_authenticator_depth3());
}

} // namespace

int main(int argc, char **argv)
{
    // /!\ WARNING: Do once for all tests. Do not
    // forget to do this !!!!
    pp::init_public_params();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
