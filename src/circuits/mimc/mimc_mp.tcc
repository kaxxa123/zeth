// DISCLAIMER:
// Content taken and adapted from:
// https://github.com/HarryR/ethsnarks/blob/master/src/gadgets/mimc.hpp

#ifndef __ZETH_MIMC_MP_TCC__
#define __ZETH_MIMC_MP_TCC__

namespace libzeth {

template<typename FieldT>
MiMC_mp_gadget<FieldT>::MiMC_mp_gadget(
    libsnark::protoboard<FieldT> &pb,
    const libsnark::pb_variable<FieldT> x,
    const libsnark::pb_variable<FieldT> y,
    const std::string &annotation_prefix
  ) :
    libsnark::gadget<FieldT>(pb, annotation_prefix),
    x(x),
    y(y),
    permutation_gadget(pb, x, y, FMT(annotation_prefix, ".cipher"))
  {
    // Allocates output variables array
    output.allocate(pb, FMT(annotation_prefix, ".output"));

    const libsnark::pb_variable<FieldT>& m = x;
    const libsnark::pb_variable<FieldT>& round_key = y;
  }

template<typename FieldT>
void MiMC_mp_gadget<FieldT>::generate_r1cs_constraints (){

    // Setting constraints for the permutation gadget
    permutation_gadget.generate_r1cs_constraints();

    const libsnark::pb_variable<FieldT>& m = x;
    const libsnark::pb_variable<FieldT>& key = y;

    // Adding constraint for the Miyaguchi-Preneel equation
    this->pb.add_r1cs_constraint(
        libsnark::r1cs_constraint<FieldT>(
          permutation_gadget.result() + m + key,
          1,
          output),
        ".out = k + E_k(m_i) + m_i");
}

template<typename FieldT>
void MiMC_mp_gadget<FieldT>::generate_r1cs_witness () const {

    // Generating witness for each permutation gadget (except last one)
    permutation_gadget.generate_r1cs_witness();

    const FieldT round_key = this->pb.val(y);

    // Filling output variables for Miyaguchi-Preenel equation
    this->pb.val(output) = round_key + this->pb.val(permutation_gadget.result()) + this->pb.val(x);

}

template<typename FieldT>
const libsnark::pb_variable<FieldT>& MiMC_mp_gadget<FieldT>::result() const {
    // Returns the output
    return output;
}

// Returns the hash of two elements
template<typename FieldT>
FieldT MiMC_mp_gadget<FieldT>::get_hash(const FieldT x, FieldT y){
    libsnark::protoboard<FieldT> pb;

    // Allocates and fill with the x and y
    libsnark::pb_variable<FieldT> pb_x;

    pb_x.allocate(pb, "x");
    pb.val(pb_x) = x;

    libsnark::pb_variable<FieldT> pb_y;

    pb_y.allocate(pb, "y");
    pb.val(pb_y) = y;

    // Initialize the Hash
    MiMC_mp_gadget<FieldT> mimc_hasher(pb, pb_x, pb_y, "mimc_hash");

    // Computes the hash
    mimc_hasher.generate_r1cs_witness();

    // Returns the hash
    return pb.val(mimc_hasher.result());
}

}  // libzeth

#endif // __ZETH_MIMC_MP_TCC__
