# Copyright (c) 2015-2021 Clearmatics Technologies Ltd
#
# SPDX-License-Identifier: LGPL-3.0+

from test_commands import mock
from unittest import TestCase
from typing import Any

# Data generated by libzeth/tests/core/ec_operation_data_test. Statements to be
# checked match those in that test.


# Internal convenience function to create an EVM bytes32
def _b32(hex_value: str) -> bytes:
    assert len(hex_value) == 64
    return bytes.fromhex(hex_value)


# BW6-761 test values

FR_MINUS_2 = [
    _b32("0000000000000000000000000000000001ae3a4617c510eac63b05c06ca1493b"),
    _b32("1a22d9f300f5138f1ef3622fba094800170b5d44300000008508bfffffffffff"),
]

G1_2 = [
    _b32("00bdd3187c4a57477dd0830d8bb83a85593798ea1a55668c8ecba3db496e132a"),
    _b32("1dd339c5fcb2cefd718d5a50f4083d3b410e83135fec7197210145ae4ddce934"),
    _b32("ec0888cb1a408ae8288edb780c1e18371da1be3a02b2f487bfa7095e760be81a"),
    _b32("009413f554540d560317eec4f050678f69354e9e935feee8baadbf7a2ec00403"),
    _b32("9163ac1bf31a15f64a820dc5ae9b84c818b321d8db0883bf4861a68212b8aa03"),
    _b32("ab23a88b0115e974e18db5c0970deb0e7130dba6f54da5179dc19db66d2fdf2a"),
]
G1_3 = [
    _b32("00ad4c3f7294388e5fc6efee4fe3453ff23479311d42a7252d3bf4c7c9b4227d"),
    _b32("ddb937dedd5dad407dc243967ef25d38d6e8e31f8692ddd3df98af5e7e0d9776"),
    _b32("5a551109795fac68ecf407d6e11ef6f3b1067366fc51e3be4a6102a57f98ad76"),
    _b32("00f596f9e74dafd5dbc4ff19e4029e145329a084b9c7f2a6e9d11269b03dc808"),
    _b32("de4453b200eda5577079dc5befe40303f4daeb118fc86bc62066e5cac871a7c9"),
    _b32("f33812d549493a2ffafbd8a06079ad32e1e3a1f93b95852bededf9d73fbb8c27"),
]
G1_4 = [
    _b32("009b512f473767b3e84f621c1efb9a875b0ae1d623dd0ac1fd29354688311902"),
    _b32("fcad849275945122751e5b564e8ffd4939174982282da32feb69c2fa8d9c32d9"),
    _b32("2f8e3825f390bf75fc4a8554b2c1367f1a4d92ff40539b4bfc2cba0298b46a82"),
    _b32("0081b87ff3a0887d2f892a8612e47d89b11140beaccdfc5f269fe046da6355ea"),
    _b32("4a954be341b294ff4ea27bc8926192867adde574fc5ee4310bc502c42f297601"),
    _b32("f172a930d1e62b8a05ce451ede1a5d4ea67f8ecd1c49ad47388257f60a7dc91a"),
]
G1_6 = [
    _b32("00760cbf3c77666f2cba2ffb4401e3830697a50fe7a46c2f977b37cb5426a6b6"),
    _b32("cc8b6490bff2cf4562cb257e40f125d63f2a6253191df6dfed26c3e04ea99fd3"),
    _b32("1ce4f347362471546de61475ea28dfaffae215beca593115e51f11d5590ce443"),
    _b32("00e01244b4533b8aef899bbc446a8b772a20b1cbd837226f41667505467dcbc7"),
    _b32("6606a730e8f55a651e3e5ffb15f213d2def6835ab538138092a86d1b27f56c42"),
    _b32("483645bdf02337291594523fe6f46a7890e31fc1a527a3fffd21fb31e735da5e"),
]
G1_MINUS_8 = [
    _b32("00c7c9438e7e51aa9360612e3cedb297517ebd7a071571b771d86f68c9ec1b28"),
    _b32("0cbcccffdb49ce6e9f77adfa85aae465d0d3c60eec959a99e296042bb6522505"),
    _b32("a25a4b9ac5a5d224d1ed2c9f6644ab31d68796d3cdf6f3b8ece3f7d4b4054f45"),
    _b32("00c928123944451fa0883338f0b276d15d9611296f0e7a91917dbfd26ee41ef9"),
    _b32("d78804ff89c3227e0551f137336da94c2fdffae9278891edf276515b2290d5b1"),
    _b32("28bb85601e4ec30ad02d4029376847c58934f3708b6f7e23142602a313f68c33"),
]

G2_4 = [
    _b32("01097dd101de6bf3b912733c275cb45229472010d95a11ca7233143700b53c0a"),
    _b32("efbf4735dc01205b0ec034adde5a4a2bb4a7d4f8ed017002cb57b465a21f3d20"),
    _b32("7f55dacbcf557822f0bdddc84441cccbf78361cf3a392f4097290be8b495059c"),
    _b32("00b57e703ff7a1dbd38c2573e6f09a85c48ebea4ad6734b7a6bed68f1565b1f9"),
    _b32("80036420c5b80d550bb785b915492f8a3db71c06939d283fe910e62332228b96"),
    _b32("57d659fa4836afac6ff16c84641d8989e7ef6f159e2f7697470a44ecbf8005aa"),
]
G2_8 = [
    _b32("0099155657cfbc579c893f9052bc8e431718c6aaf22583ff79d22b3eb30daaa3"),
    _b32("1ea7def53f90c5719c38599490604cf0447c30b495723f16468a4c4cbcdcf3be"),
    _b32("3564274269ddd9e0b376d98ddd1cae3293eeeb7cfe1e0b937db9e60b56c5f6b1"),
    _b32("011a9fcd05529c46d7d5c226edea31829dd897465f3a4a2773cb88179324ccff"),
    _b32("6b0d4bf3dd95bbeb000db16f27cee591f949aa6f6df17cec9a63dc660f4aa79f"),
    _b32("ed5512a98239b73fdff458e7c0e09b9d246e3f5681429affc2b1f7e2fcbd1141"),
]

BW6_INSTANCE: Any = None


class TestBW6_761Contract(TestCase):

    @staticmethod
    def setUpClass() -> None:
        print("Deploying TestBW6_761.sol")
        _web3, eth = mock.open_test_web3()
        _bw6_interface, bw6_instance = mock.deploy_contract(
            eth,
            eth.accounts[0],
            "TestBW6_761",
            {})
        global BW6_INSTANCE  # pylint: disable=global-statement
        BW6_INSTANCE = bw6_instance

    def test_bw6_ecadd(self) -> None:
        """
        Check that [6] == [2] + [4]
        """
        result = BW6_INSTANCE.functions.testECAdd(G1_2 + G1_4).call()
        self.assertEqual(G1_6, result)

    def test_bw6_ecmul(self) -> None:
        """
        Check that [-8] == -2 * [4]
        """
        result = BW6_INSTANCE.functions.testECMul(G1_4 + FR_MINUS_2).call()
        self.assertEqual(G1_MINUS_8, result)

    def test_bw6_ecpairing(self) -> None:
        """
        Check that e([6], [4]) * e([3],[8]) * e([4],[4]) * e([-8], [8]) == 1
        """
        # Note, return result here is uint256(1) or uint256(0) depending on the
        # pairing check result.
        points = G1_6 + G2_4 + G1_3 + G2_8 + G1_4 + G2_4 + G1_MINUS_8 + G2_8
        result = BW6_INSTANCE.functions.testECPairing(points).call()
        self.assertEqual(1, result)

        points = G1_6 + G2_4 + G1_3 + G2_8 + G1_4 + G2_4 + G1_MINUS_8 + G2_4
        result = BW6_INSTANCE.functions.testECPairing(points).call()
        self.assertEqual(0, result)
