// Copyright (c) 2015-2020 Clearmatics Technologies Ltd
//
// SPDX-License-Identifier: LGPL-3.0+

#include "libzeth/circuit_types.hpp"
#include "libzeth/libsnark_helpers/libsnark_helpers.hpp"
#include "libzeth/snarks_alias.hpp"
#include "libzeth/util.hpp"
#include "libzeth/util_api.hpp"
#include "libzeth/zeth.h"
#include "zethConfig.h"

#include <boost/program_options.hpp>
#include <fstream>
#include <grpc/grpc.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>
#include <grpcpp/server_context.h>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>

// Necessary header to parse the data
#include <libsnark/common/data_structures/merkle_tree.hpp>

// Include the file generated by gRPC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "api/prover.grpc.pb.h"
#pragma GCC diagnostic pop

// Include the API for the given SNARK
#include "libzeth/snarks_api_imports.hpp"

namespace proto = google::protobuf;
namespace po = boost::program_options;

/// The prover_server class inherits from the Prover service
/// defined in the proto files, and provides an implementation
/// of the service.
class prover_server final : public zeth_proto::Prover::Service
{
private:
    libzeth::circuit_wrapper<
        libzeth::FieldT,
        libzeth::HashT,
        libzeth::HashTreeT,
        libzeth::ppT,
        libzeth::ZETH_NUM_JS_INPUTS,
        libzeth::ZETH_NUM_JS_OUTPUTS,
        libzeth::ZETH_MERKLE_TREE_DEPTH>
        prover;

    // The keypair is the result of the setup
    libzeth::keyPairT<libzeth::ppT> keypair;

public:
    explicit prover_server(
        libzeth::circuit_wrapper<
            libzeth::FieldT,
            libzeth::HashT,
            libzeth::HashTreeT,
            libzeth::ppT,
            libzeth::ZETH_NUM_JS_INPUTS,
            libzeth::ZETH_NUM_JS_OUTPUTS,
            libzeth::ZETH_MERKLE_TREE_DEPTH> &prover,
        libzeth::keyPairT<libzeth::ppT> &keypair)
        : prover(prover), keypair(keypair)
    {
    }

    grpc::Status GetVerificationKey(
        grpc::ServerContext *,
        const proto::Empty *,
        zeth_proto::VerificationKey *response) override
    {
        std::cout << "[ACK] Received the request to get the verification key"
                  << std::endl;
        std::cout << "[DEBUG] Preparing verification key for response..."
                  << std::endl;
        try {
            libzeth::prepare_verification_key_response<libzeth::ppT>(
                this->keypair.vk, response);
        } catch (const std::exception &e) {
            std::cout << "[ERROR] " << e.what() << std::endl;
            return grpc::Status(
                grpc::StatusCode::INVALID_ARGUMENT, grpc::string(e.what()));
        } catch (...) {
            std::cout << "[ERROR] In catch all" << std::endl;
            return grpc::Status(grpc::StatusCode::UNKNOWN, "");
        }

        return grpc::Status::OK;
    }

    grpc::Status Prove(
        grpc::ServerContext *,
        const zeth_proto::ProofInputs *proof_inputs,
        zeth_proto::ExtendedProof *proof) override
    {
        std::cout << "[ACK] Received the request to generate a proof"
                  << std::endl;
        std::cout << "[DEBUG] Parse received message to compute proof..."
                  << std::endl;

        // Parse received message to feed to the prover
        try {
            libzeth::FieldT root =
                libzeth::hexadecimal_str_to_field_element<libzeth::FieldT>(
                    proof_inputs->mk_root());
            libzeth::bits64 vpub_in =
                libzeth::hex_value_to_bits64(proof_inputs->pub_in_value());
            libzeth::bits64 vpub_out =
                libzeth::hex_value_to_bits64(proof_inputs->pub_out_value());
            libzeth::bits256 h_sig_in =
                libzeth::hex_digest_to_bits256(proof_inputs->h_sig());
            libzeth::bits256 phi_in =
                libzeth::hex_digest_to_bits256(proof_inputs->phi());

            if (libzeth::ZETH_NUM_JS_INPUTS != proof_inputs->js_inputs_size()) {
                throw std::invalid_argument("Invalid number of JS inputs");
            }
            if (libzeth::ZETH_NUM_JS_OUTPUTS !=
                proof_inputs->js_outputs_size()) {
                throw std::invalid_argument("Invalid number of JS outputs");
            }

            std::cout << "[DEBUG] Process all inputs of the JoinSplit"
                      << std::endl;
            std::array<
                libzeth::joinsplit_input<
                    libzeth::FieldT,
                    libzeth::ZETH_MERKLE_TREE_DEPTH>,
                libzeth::ZETH_NUM_JS_INPUTS>
                joinsplit_inputs;
            for (size_t i = 0; i < libzeth::ZETH_NUM_JS_INPUTS; i++) {
                printf(
                    "\r  input (%zu / %zu)\n", i, libzeth::ZETH_NUM_JS_INPUTS);
                zeth_proto::JoinsplitInput received_input =
                    proof_inputs->js_inputs(i);
                libzeth::joinsplit_input<
                    libzeth::FieldT,
                    libzeth::ZETH_MERKLE_TREE_DEPTH>
                    parsed_input = libzeth::parse_joinsplit_input<
                        libzeth::FieldT,
                        libzeth::ZETH_MERKLE_TREE_DEPTH>(received_input);
                joinsplit_inputs[i] = parsed_input;
            }

            std::cout << "[DEBUG] Process all outputs of the JoinSplit"
                      << std::endl;
            std::array<libzeth::zeth_note, libzeth::ZETH_NUM_JS_OUTPUTS>
                joinsplit_outputs;
            for (size_t i = 0; i < libzeth::ZETH_NUM_JS_OUTPUTS; i++) {
                printf(
                    "\r  output (%zu / %zu)\n",
                    i,
                    libzeth::ZETH_NUM_JS_OUTPUTS);
                zeth_proto::ZethNote received_output =
                    proof_inputs->js_outputs(i);
                libzeth::zeth_note parsed_output =
                    libzeth::parse_zeth_note(received_output);
                joinsplit_outputs[i] = parsed_output;
            }

            std::cout << "[DEBUG] Data parsed successfully" << std::endl;
            std::cout << "[DEBUG] Generating the proof..." << std::endl;
            libzeth::extended_proof<libzeth::ppT> ext_proof =
                this->prover.prove(
                    root,
                    joinsplit_inputs,
                    joinsplit_outputs,
                    vpub_in,
                    vpub_out,
                    h_sig_in,
                    phi_in,
                    this->keypair.pk);

            std::cout << "[DEBUG] Displaying the extended proof" << std::endl;
            ext_proof.dump_proof();
            ext_proof.dump_primary_inputs();

            std::cout << "[DEBUG] Preparing response..." << std::endl;
            libzeth::prepare_proof_response<libzeth::ppT>(ext_proof, proof);

        } catch (const std::exception &e) {
            std::cout << "[ERROR] " << e.what() << std::endl;
            return grpc::Status(
                grpc::StatusCode::INVALID_ARGUMENT, grpc::string(e.what()));
        } catch (...) {
            std::cout << "[ERROR] In catch all" << std::endl;
            return grpc::Status(grpc::StatusCode::UNKNOWN, "");
        }

        return grpc::Status::OK;
    }
};

std::string get_server_version()
{
    char buffer[100];
    int n;
    // Defined in the zethConfig file
    n = snprintf(
        buffer, 100, "Version %d.%d", ZETH_VERSION_MAJOR, ZETH_VERSION_MINOR);
    if (n < 0) {
        return "Version <Not specified>";
    }
    std::string version(buffer);
    return version;
}

void display_server_start_message()
{
    std::string copyright =
        "Copyright (c) 2015-2020 Clearmatics Technologies Ltd";
    std::string license = "SPDX-License-Identifier: LGPL-3.0+";
    std::string project =
        "R&D Department: PoC for Zerocash on Ethereum/Autonity";
    std::string version = get_server_version();
    std::string warning = "**WARNING:** This code is a research-quality proof "
                          "of concept, DO NOT use in production!";

    std::cout << "\n=====================================================\n";
    std::cout << copyright << "\n";
    std::cout << license << "\n";
    std::cout << project << "\n";
    std::cout << version << "\n";
    std::cout << warning << "\n";
    std::cout << "=====================================================\n"
              << std::endl;
}

static void RunServer(
    libzeth::circuit_wrapper<
        libzeth::FieldT,
        libzeth::HashT,
        libzeth::HashTreeT,
        libzeth::ppT,
        libzeth::ZETH_NUM_JS_INPUTS,
        libzeth::ZETH_NUM_JS_OUTPUTS,
        libzeth::ZETH_MERKLE_TREE_DEPTH> &prover,
    libzeth::keyPairT<libzeth::ppT> &keypair)
{
    // Listen for incoming connections on 0.0.0.0:50051
    std::string server_address("0.0.0.0:50051");

    prover_server service(prover, keypair);

    grpc::ServerBuilder builder;

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);

    // Finally assemble the server.
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "[DEBUG] Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    display_server_start_message();
    server->Wait();
}

#ifdef ZKSNARK_GROTH16
static libzeth::keyPairT<libzeth::ppT> load_keypair(
    const std::string &keypair_file)
{
    std::ifstream in(keypair_file, std::ios_base::in | std::ios_base::binary);
    in.exceptions(
        std::ios_base::eofbit | std::ios_base::badbit | std::ios_base::failbit);
    return libzeth::mpc_read_keypair<libzeth::ppT>(in);
}
#endif

int main(int argc, char **argv)
{
    // Options
    po::options_description options("");
    options.add_options()(
        "keypair,k", po::value<std::string>(), "file to load keypair from");
#ifdef DEBUG
    options.add_options()(
        "jr1cs,j",
        po::value<boost::filesystem::path>(),
        "file in which to export the r1cs in json format");
#endif

    auto usage = [&]() {
        std::cout << "Usage:"
                  << "\n"
                  << "  " << argv[0] << " [<options>]\n"
                  << "\n";
        std::cout << options;
        std::cout << std::endl;
    };

    std::string keypair_file;
#ifdef DEBUG
    boost::filesystem::path jr1cs_file;
#endif
    try {
        po::variables_map vm;
        po::store(
            po::command_line_parser(argc, argv).options(options).run(), vm);
        if (vm.count("help")) {
            usage();
            return 0;
        }
        if (vm.count("keypair")) {
            keypair_file = vm["keypair"].as<std::string>();
        }
#ifdef DEBUG
        if (vm.count("jr1cs")) {
            jr1cs_file = vm["jr1cs"].as<boost::filesystem::path>();
        }
#endif
    } catch (po::error &error) {
        std::cerr << " ERROR: " << error.what() << std::endl;
        usage();
        return 1;
    }

    // We inititalize the curve parameters here
    std::cout << "[INFO] Init params" << std::endl;
    libzeth::ppT::init_public_params();

    libzeth::circuit_wrapper<
        libzeth::FieldT,
        libzeth::HashT,
        libzeth::HashTreeT,
        libzeth::ppT,
        libzeth::ZETH_NUM_JS_INPUTS,
        libzeth::ZETH_NUM_JS_OUTPUTS,
        libzeth::ZETH_MERKLE_TREE_DEPTH>
        prover;
    libzeth::keyPairT<libzeth::ppT> keypair = [&keypair_file, &prover]() {
        if (!keypair_file.empty()) {
#ifdef ZKSNARK_GROTH16
            std::cout << "[INFO] Loading keypair: " << keypair_file
                      << std::endl;
            return load_keypair(keypair_file);
#else
            std::cout << "Keypair loading not supported in this config"
                      << std::endl;
            exit(1);
#endif
        }

        std::cout << "[INFO] Generate new keypair" << std::endl;
        return prover.generate_trusted_setup();
    }();

#ifdef DEBUG
    // Run only if the flag is set
    if (jr1cs_file != "") {
        std::cout << "[DEBUG] Dump R1CS to json file" << std::endl;
        prover.dump_constraint_system(jr1cs_file);
    }
#endif

    std::cout << "[INFO] Setup successful, starting the server..." << std::endl;
    RunServer(prover, keypair);
    return 0;
}
