/**
 * @generated by contrib/devtools/chainparams/generate_chainparams_constants.py
 */

#include <chainparamsconstants.h>

namespace ChainParamsConstants {
    const BlockHash MAINNET_DEFAULT_ASSUME_VALID = BlockHash::fromHex("00000000000000000c11c4910b3a301ae1a9c76fd3edbba2a41cffab8397e683");
    const uint256 MAINNET_MINIMUM_CHAIN_WORK = uint256S("00000000000000000000000000000000000000000167b6b46be14822263ea7de");
    const uint64_t MAINNET_ASSUMED_BLOCKCHAIN_SIZE = 211;
    const uint64_t MAINNET_ASSUMED_CHAINSTATE_SIZE = 3;

    const BlockHash TESTNET_DEFAULT_ASSUME_VALID = BlockHash::fromHex("000000000005fec2e6827841ea4a4d856009b41184fc15cce87166e93faaec85");
    const uint256 TESTNET_MINIMUM_CHAIN_WORK = uint256S("00000000000000000000000000000000000000000000006e9cadbd73b0c2d92b");
    const uint64_t TESTNET_ASSUMED_BLOCKCHAIN_SIZE = 55;
    const uint64_t TESTNET_ASSUMED_CHAINSTATE_SIZE = 2;
} // namespace ChainParamsConstants

