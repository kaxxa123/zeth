#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

//hash
//#include <libsnark/gadgetlib1/gadgets/hashes/sha256/sha256_gadget.hpp>
#include <sha256/sha256_ethereum.cpp>
#include <export.cpp>

//key gen 
#include "libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp" //hold key

#include <libsnark/common/data_structures/merkle_tree.hpp>

#include <libsnark/gadgetlib1/gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/merkle_tree/merkle_authentication_path_variable.hpp>
#include <libsnark/gadgetlib1/gadgets/merkle_tree/merkle_tree_check_read_gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/merkle_tree/merkle_tree_check_update_gadget.hpp>

// We do this to avoid using namespace prefixes libsnark:: and libff::
// everytime we want to invoke something implemented in libsnark or libff
// respectively
using namespace libsnark;
using namespace libff;

// These variables are taken from the deploy.js script
// The node variables represent the values of the nodes of the merkle tree, which is a bytes32[32], and which contains 16 leaves
// starting at index 16 - 31 in the bytes32[32] array

// TODO: Deal with leading zeros being removed in deploy.js

libff::bit_vector node17 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

libff::bit_vector node16 = { 1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 };

libff::bit_vector node9 = { 1 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 };

libff::bit_vector node5 = { 1 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,1 };

libff::bit_vector node3 = { 1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 };

libff::bit_vector node_root = {0, 1 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0};

libff::bit_vector nullifier = {0,0, 1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,0 };

libff::bit_vector secret = { 1 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,1 ,0 ,0 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,0 ,0 ,1 ,0 ,1 ,1 ,0 ,1 ,0 ,1 ,1 ,1 ,1 ,0 ,0 ,1 ,1 ,1 ,0 ,0 ,0 ,1 ,1 ,1 ,0 ,1 ,1 ,0 ,1 ,1 };

template<typename FieldT, typename HashT>
class Miximus {
    public:
        const size_t digest_len = HashT::get_digest_len();
        const size_t tree_depth = 4;

        /**
         *
         * See https://github.com/scipr-lab/libsnark/blob/master/libsnark/gadgetlib2/examples/tutorial.cpp
         * for more details about the protoboard. It says that:
         * - The protoboard is the 'memory manager' which holds all constraints 
         * - (when creating the verifying circuit) and variable assignments (when creating the proof witness). 
         * - We specify the type as R1P, this can be augmented in the future to allow for BOOLEAN 
         * - or GF2_EXTENSION fields in the future.
         *
         * The line "protoboard<FieldT>" takes origin here: https://github.com/scipr-lab/libsnark/blob/92a80f74727091fdc40e6021dc42e9f6b67d5176/libsnark/gadgetlib1/protoboard.hpp#L30-L31
         * And makes use of the Cpp templates. Then, here we build a protoboard (we call the constructor)
         * by giving a type FieldT in the template.
         *
         */
        protoboard<FieldT> pb;

        std::shared_ptr<multipacking_gadget<FieldT>> unpacker;
        std::shared_ptr<multipacking_gadget<FieldT>> unpacker1;

        std::shared_ptr<digest_variable<FieldT>> root_digest;
        std::shared_ptr<digest_variable<FieldT>> cm;
        std::shared_ptr<digest_variable<FieldT>> sk;
        std::shared_ptr<digest_variable<FieldT>> leaf_digest;

        std::shared_ptr<sha256_ethereum> cm_hash;

        std::shared_ptr<merkle_authentication_path_variable<FieldT, HashT>> path_variable;
        // See: libsnark/gadgetlib1/gadgets/merkle_tree/merkle_tree_check_read_gadget.hpp
        // The gadget checks the following: given a root R, address A, value V, and
        // authentication path P, check that P is a valid authentication path for the
        // value V as the A-th leaf in a Merkle tree with root R.
        std::shared_ptr<merkle_tree_check_read_gadget<FieldT, HashT>> check_membership;

        pb_variable_array<FieldT> address_bits_va;
        std::shared_ptr <block_variable<FieldT>> input_variable;
        pb_variable<FieldT> ZERO;

        // I think there are 2 variable arrays because we want to prove 2 things
        // 1) Bob knows the sk
        // 2) The leaf associated with the sk is in the tree
        // TODO: Verify if I'm right or not
        pb_variable_array<FieldT> packed_inputs;
        pb_variable_array<FieldT> unpacked_inputs;

        pb_variable_array<FieldT> packed_inputs1;
        pb_variable_array<FieldT> unpacked_inputs1;

        // Constructor of the class "Miximus"
        Miximus() {
            packed_inputs.allocate(pb, 1 + 1, "packed");
            packed_inputs1.allocate(pb, 1 + 1, "packed");

            ZERO.allocate(pb, "ZERO");
            pb.val(ZERO) = 0;
            address_bits_va.allocate(pb, tree_depth, "address_bits");

            cm.reset(new digest_variable<FieldT>(pb, 256, "cm"));
            root_digest.reset(new digest_variable<FieldT>(pb, 256, "root_digest"));
            sk.reset(new digest_variable<FieldT>(pb, 256, "sk"));
            leaf_digest.reset(new digest_variable<FieldT>(pb, 256, "leaf_digest"));

            unpacked_inputs.insert(unpacked_inputs.end(), root_digest->bits.begin(), root_digest->bits.end());
            unpacker.reset(new multipacking_gadget<FieldT>(pb, unpacked_inputs, packed_inputs, FieldT::capacity(), "unpacker"));

            unpacked_inputs1.insert(unpacked_inputs1.end(), cm->bits.begin(), cm->bits.end());
            unpacker1.reset(new multipacking_gadget<FieldT>(pb, unpacked_inputs1, packed_inputs1, FieldT::capacity(), "unpacker"));

            pb.set_input_sizes(18 + 1); // The size of the inputs is: 16 (leaves) + root + sk + cm (TODO: To verify)

            input_variable.reset(new block_variable<FieldT>(pb, *cm, *sk, "input_variable")); 

            cm_hash.reset(new sha256_ethereum(pb, SHA256_block_size, *input_variable, *leaf_digest, "cm_hash"));
            path_variable.reset(new  merkle_authentication_path_variable<FieldT, HashT> (pb, tree_depth, "path_variable" ));
            // merkle_authentication_path_variable<FieldT, HashT> path_variable(pb, tree_depth, "path_variable");
            check_membership.reset(new merkle_tree_check_read_gadget<FieldT, HashT>(pb, tree_depth, address_bits_va, *leaf_digest, *root_digest, *path_variable, ONE, "check_membership"));

            // Generate constraints
            // root_digest.generate_r1cs_constraints();
            unpacker->generate_r1cs_constraints(true);
            unpacker1->generate_r1cs_constraints(false);

            generate_r1cs_equals_const_constraint<FieldT>(pb, ZERO, FieldT::zero(), "ZERO");
            cm_hash->generate_r1cs_constraints(true);
            path_variable->generate_r1cs_constraints();
            check_membership->generate_r1cs_constraints();
            leaf_digest->generate_r1cs_constraints();
        }

        // The purpose of this cpp program is to generate the proof that is going to be verified
        // on the solidity contract deployed on the blockchain
        void prove() { 
            // generate witness
            // unpacker->generate_r1cs_constraints(false);
            std::vector<merkle_authentication_node> path(tree_depth);

            libff::bit_vector leaf = node16; // The proof is given for the node16 (containing the inserted commitment)
            libff::bit_vector address_bits;
            size_t address = 0; // uint of address_bits
            address_bits = {0,0,0,0}; // defines the ordering of hashing
            path = {node3,node5,node9,node17}; // path is declared as a vector of length tree_depth (= 4, here), of merkle tree nodes

            cm->generate_r1cs_witness(nullifier);
            root_digest->generate_r1cs_witness(node_root);
            sk->generate_r1cs_witness(secret);
            cm_hash->generate_r1cs_witness();  
            //leaf_digest->generate_r1cs_witness(leaf);
            path_variable->generate_r1cs_witness(address, path);
            check_membership->generate_r1cs_witness();
            unpacker->generate_r1cs_witness_from_bits();
            unpacker1->generate_r1cs_witness_from_bits();

            // unpacker->generate_r1cs_witness_from_packed();
            address_bits_va.fill_with_bits(pb, address_bits);
            assert(address_bits_va.get_field_element_from_bits(pb).as_ulong() == address);

            // make sure that read checker didn't accidentally overwrite anything 
            address_bits_va.fill_with_bits(pb, address_bits);
            unpacker->generate_r1cs_witness_from_bits();
            leaf_digest->generate_r1cs_witness(leaf);
            root_digest->generate_r1cs_witness(node_root);

            assert(pb.is_satisfied());
            std::cout << "is satisfied" << pb.is_satisfied() << "\n";

            // const size_t num_constraints = pb.num_constraints();
            // const size_t expected_constraints = merkle_tree_check_read_gadget<FieldT, HashT>::expected_constraints(tree_depth);
            dump_key(pb, "out.txt");
            // assert(num_constraints == expected_constraints);
        }
};

template<typename ppT>
void test_all_merkle_tree_gadgets() {
    typedef libff::Fr<ppT> FieldT;
    // test();
    // main_merkle_tree_check_read_gadget<FieldT, sha256_ethereum>();
    Miximus<FieldT, sha256_ethereum> prover;
    prover.prove();
}

int main () {
    // See: https://github.com/scipr-lab/libff/blob/master/libff/algebra/curves/alt_bn128/alt_bn128_init.cpp
    libff::alt_bn128_pp::init_public_params();
    test_all_merkle_tree_gadgets<libff::alt_bn128_pp>();
    return 0;
}
