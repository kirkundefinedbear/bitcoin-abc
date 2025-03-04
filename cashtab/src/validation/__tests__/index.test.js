// Copyright (c) 2024 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

import {
    isValidTokenName,
    isValidTokenTicker,
    isValidTokenDecimals,
    isValidTokenDocumentUrl,
    isValidCashtabSettings,
    isValidNewWalletNameLength,
    isValidXecSendAmount,
    isValidTokenId,
    isValidXecAirdrop,
    isValidAirdropExclusionArray,
    isValidContactList,
    migrateLegacyCashtabSettings,
    isValidCashtabCache,
    validateMnemonic,
    meetsAliasSpec,
    isValidAliasSendInput,
    isProbablyNotAScam,
    isValidRecipient,
    isValidSideshiftObj,
    isValidMultiSendUserInput,
    shouldSendXecBeDisabled,
    parseAddressInput,
    isValidCashtabWallet,
    isValidTokenSendOrBurnAmount,
    isValidTokenMintAmount,
    getOpReturnRawError,
    nodeWillAcceptOpReturnRaw,
} from 'validation';
import {
    validXecAirdropExclusionList,
    invalidXecAirdropExclusionList,
    invalidXecAirdropExclusionListPrefixless,
} from 'validation/fixtures/mocks';
import vectors from 'validation/fixtures/vectors';
import { when } from 'jest-when';
import appConfig from 'config/app';
import aliasSettings from 'config/alias';

describe('Cashtab validation functions', () => {
    it(`isValidSideshiftObj() returns true for a valid sideshift library object`, () => {
        const mockSideshift = {
            show: () => {
                return true;
            },
            hide: () => {
                return true;
            },
            addEventListener: () => {
                return true;
            },
        };
        expect(isValidSideshiftObj(mockSideshift)).toBe(true);
    });
    it(`isValidSideshiftObj() returns false if the sideshift library object failed to instantiate`, () => {
        expect(isValidSideshiftObj(null)).toBe(false);
    });
    it(`isValidSideshiftObj() returns false for an invalid sideshift library object`, () => {
        const mockSideshift = {
            show: () => {
                return true;
            },
            hide: () => {
                return true;
            },
            addEvenListener: 'not-a-function',
        };
        expect(isValidSideshiftObj(mockSideshift)).toBe(false);
    });
    it(`isValidRecipient() returns true for a valid and registered alias input`, async () => {
        const mockRegisteredAliasResponse = {
            alias: 'cbdc',
            address: 'ecash:qq9h6d0a5q65fgywv4ry64x04ep906mdku8f0gxfgx',
            txid: 'f7d71433af9a4e0081ea60349becf2a60efed8890df7c3e8e079b3427f51d5ea',
            blockheight: 802515,
        };
        const fetchUrl = `${aliasSettings.aliasServerBaseUrl}/alias/${mockRegisteredAliasResponse.alias}`;

        // mock the fetch call to alias-server's '/alias' endpoint
        global.fetch = jest.fn();
        when(fetch)
            .calledWith(fetchUrl)
            .mockResolvedValue({
                json: () => Promise.resolve(mockRegisteredAliasResponse),
            });
        expect(await isValidRecipient('cbdc.xec')).toBe(true);
    });
    it(`isValidRecipient() returns false for a valid but unregistered alias input`, async () => {
        const mockUnregisteredAliasResponse = {
            alias: 'koush',
            isRegistered: false,
            registrationFeeSats: 554,
            processedBlockheight: 803421,
        };
        const fetchUrl = `${aliasSettings.aliasServerBaseUrl}/alias/${mockUnregisteredAliasResponse.alias}`;

        // mock the fetch call to alias-server's '/alias' endpoint
        global.fetch = jest.fn();
        when(fetch)
            .calledWith(fetchUrl)
            .mockResolvedValue({
                json: () => Promise.resolve(mockUnregisteredAliasResponse),
            });
        expect(await isValidRecipient('koush.xec')).toBe(false);
    });
    it(`isValidRecipient() returns false for an invalid eCash address / alias input`, async () => {
        expect(await isValidRecipient('notvalid')).toBe(false);
    });
    it(`isValidRecipient() returns true for a valid eCash address`, async () => {
        expect(
            await isValidRecipient(
                'ecash:qq9h6d0a5q65fgywv4ry64x04ep906mdku8f0gxfgx',
            ),
        ).toBe(true);
    });
    it(`isValidRecipient() returns true for a valid prefix-less eCash address`, async () => {
        expect(
            await isValidRecipient(
                'qq9h6d0a5q65fgywv4ry64x04ep906mdku8f0gxfgx',
            ),
        ).toBe(true);
    });
    it(`validateMnemonic() returns true for a valid mnemonic`, () => {
        const mnemonic =
            'labor tail bulb distance estate collect lecture into smile differ yard legal';
        expect(validateMnemonic(mnemonic)).toBe(true);
    });
    it(`validateMnemonic() returns false for an invalid mnemonic`, () => {
        const mnemonic =
            'labor tail bulb not valid collect lecture into smile differ yard legal';
        expect(validateMnemonic(mnemonic)).toBe(false);
    });
    it(`validateMnemonic() returns false for an empty string mnemonic`, () => {
        const mnemonic = '';
        expect(validateMnemonic(mnemonic)).toBe(false);
    });
    it(`Accepts a valid ${appConfig.tokenTicker} token name`, () => {
        expect(isValidTokenName('Valid token name')).toBe(true);
    });
    it(`Accepts a valid ${appConfig.tokenTicker} token name that is a stringified number`, () => {
        expect(isValidTokenName('123456789')).toBe(true);
    });
    it(`Rejects ${appConfig.tokenTicker} token name if longer than 68 characters`, () => {
        expect(
            isValidTokenName(
                'This token name is not valid because it is longer than 68 characters which is really pretty long for a token name when you think about it and all',
            ),
        ).toBe(false);
    });
    it(`Rejects ${appConfig.tokenTicker} token name if empty string`, () => {
        expect(isValidTokenName('')).toBe(false);
    });
    it(`Accepts a 4-char ${appConfig.tokenTicker} token ticker`, () => {
        expect(isValidTokenTicker('DOGG')).toBe(true);
    });
    it(`Accepts a 12-char ${appConfig.tokenTicker} token ticker`, () => {
        expect(isValidTokenTicker('123456789123')).toBe(true);
    });
    it(`Rejects ${appConfig.tokenTicker} token ticker if empty string`, () => {
        expect(isValidTokenTicker('')).toBe(false);
    });
    it(`Rejects ${appConfig.tokenTicker} token ticker if > 12 chars`, () => {
        expect(isValidTokenTicker('1234567891234')).toBe(false);
    });
    it(`Accepts tokenDecimals if zero`, () => {
        expect(isValidTokenDecimals('0')).toBe(true);
    });
    it(`Accepts tokenDecimals if between 0 and 9 inclusive`, () => {
        expect(isValidTokenDecimals('9')).toBe(true);
    });
    it(`Rejects tokenDecimals if empty string`, () => {
        expect(isValidTokenDecimals('')).toBe(false);
    });
    it(`Rejects tokenDecimals if non-integer`, () => {
        expect(isValidTokenDecimals('1.7')).toBe(false);
    });
    it(`Accepts a valid ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('cashtabapp.com')).toBe(true);
    });
    it(`Accepts a valid ${appConfig.tokenTicker} token document URL including special URL characters`, () => {
        expect(isValidTokenDocumentUrl('https://cashtabapp.com/')).toBe(true);
    });
    it(`Accepts a blank string as a valid ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('')).toBe(true);
    });
    it(`Rejects ${appConfig.tokenTicker} token name if longer than 68 characters`, () => {
        expect(
            isValidTokenDocumentUrl(
                'http://www.ThisTokenDocumentUrlIsActuallyMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchMuchTooLong.com/',
            ),
        ).toBe(false);
    });
    it(`Accepts a domain input with https protocol as ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('https://google.com')).toBe(true);
    });
    it(`Accepts a domain input with http protocol as ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('http://test.com')).toBe(true);
    });
    it(`Accepts a domain input with a primary and secondary top level domain as ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('http://test.co.uk')).toBe(true);
    });
    it(`Accepts a domain input with just a subdomain as ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('www.test.co.uk')).toBe(true);
    });
    it(`Rejects a domain input with no top level domain, protocol or subdomain  ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl('mywebsite')).toBe(false);
    });
    it(`Rejects a domain input as numbers ${appConfig.tokenTicker} token document URL`, () => {
        expect(isValidTokenDocumentUrl(12345)).toBe(false);
    });
    it(`isValidTokenId accepts valid token ID that is 64 chars in length`, () => {
        const testValidTokenId =
            '1c6c9c64d70b285befe733f175d0f384538576876bd280b10587df81279d3f5e';
        expect(isValidTokenId(testValidTokenId)).toBe(true);
    });
    it(`isValidTokenId rejects a token ID that is less than 64 chars in length`, () => {
        const testValidTokenId = '111111thisisaninvalidtokenid';
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects a token ID that is more than 64 chars in length`, () => {
        const testValidTokenId =
            '111111111c6c9c64d70b285befe733f175d0f384538576876bd280b10587df81279d3f5e';
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects a token ID number that is 64 digits in length`, () => {
        // eslint-disable-next-line no-loss-of-precision
        const testValidTokenId = 8912345678912345678912345678912345678912345678912345678912345679;
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects null`, () => {
        const testValidTokenId = null;
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects undefined`, () => {
        const testValidTokenId = undefined;
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects empty string`, () => {
        const testValidTokenId = '';
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects special character input`, () => {
        const testValidTokenId = '^&$%@&^$@&%$!';
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidTokenId rejects non-alphanumeric input`, () => {
        const testValidTokenId = 99999999999;
        expect(isValidTokenId(testValidTokenId)).toBe(false);
    });
    it(`isValidXecAirdrop accepts valid Total Airdrop Amount`, () => {
        const testAirdropTotal = '1000000';
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(true);
    });
    it(`isValidXecAirdrop rejects null`, () => {
        const testAirdropTotal = null;
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(false);
    });
    it(`isValidXecAirdrop rejects undefined`, () => {
        const testAirdropTotal = undefined;
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(false);
    });
    it(`isValidXecAirdrop rejects empty string`, () => {
        const testAirdropTotal = '';
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(false);
    });
    it(`isValidTokenId rejects an alphanumeric input`, () => {
        const testAirdropTotal = 'a73hsyujs3737';
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(false);
    });
    it(`isValidTokenId rejects a number !> 0 in string format`, () => {
        const testAirdropTotal = '0';
        expect(isValidXecAirdrop(testAirdropTotal)).toBe(false);
    });
    it(`isValidAirdropExclusionArray accepts a valid airdrop exclusion list`, () => {
        expect(isValidAirdropExclusionArray(validXecAirdropExclusionList)).toBe(
            true,
        );
    });
    it(`isValidAirdropExclusionArray rejects an invalid airdrop exclusion list`, () => {
        expect(
            isValidAirdropExclusionArray(invalidXecAirdropExclusionList),
        ).toBe(false);
    });
    it(`isValidAirdropExclusionArray rejects a list of addresses if any are not prefixed`, () => {
        expect(
            isValidAirdropExclusionArray(
                invalidXecAirdropExclusionListPrefixless,
            ),
        ).toBe(false);
    });
    it(`isValidAirdropExclusionArray rejects an empty airdrop exclusion list`, () => {
        expect(isValidAirdropExclusionArray([])).toBe(false);
    });
    it(`isValidAirdropExclusionArray rejects a null airdrop exclusion list`, () => {
        expect(isValidAirdropExclusionArray(null)).toBe(false);
    });
    it(`accepts a valid wallet name`, () => {
        expect(isValidNewWalletNameLength('Apollo')).toBe(true);
    });
    it(`rejects wallet name that is too long`, () => {
        expect(
            isValidNewWalletNameLength(
                'this string is far too long to be used as a wallet name...',
            ),
        ).toBe(false);
    });
    it(`rejects blank string as new wallet name`, () => {
        expect(isValidNewWalletNameLength('')).toBe(false);
    });
    it(`rejects wallet name of the wrong type`, () => {
        expect(isValidNewWalletNameLength(['newWalletName'])).toBe(false);
    });
    it(`isProbablyNotAScam recognizes "bitcoin" is probably a scam token name`, () => {
        expect(isProbablyNotAScam('bitcoin')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes "ebitcoin" is probably a scam token name`, () => {
        expect(isProbablyNotAScam('ebitcoin')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes "Lido Staked Ether", from coingeckoTop500Names, is probably a scam token name`, () => {
        expect(isProbablyNotAScam('Lido Staked Ether')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes 'matic-network', from coingeckoTop500Ids, is probably a scam token name`, () => {
        expect(isProbablyNotAScam('matic-network')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes 'Australian Dollar', from Cashtab supported fiat currencies, is probably a scam token name`, () => {
        expect(isProbablyNotAScam('Australian Dollar')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes 'ebtc', from bannedTickers, is probably a scam token name`, () => {
        expect(isProbablyNotAScam('ebtc')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes 'gbp', from bannedTickers, is probably a scam`, () => {
        expect(isProbablyNotAScam('gbp')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes 'Hong Kong Dollar', from fiatNames, is probably a scam`, () => {
        expect(isProbablyNotAScam('gbp')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes '₪', from fiat symbols, is probably a scam`, () => {
        expect(isProbablyNotAScam('₪')).toBe(false);
    });
    it(`isProbablyNotAScam recognizes an ordinary token name as acceptable`, () => {
        expect(isProbablyNotAScam('just a normal token name')).toBe(true);
    });
    it(`isProbablyNotAScam accepts a token name with fragments of banned potential scam names`, () => {
        expect(
            isProbablyNotAScam(
                'This token is not Ethereum or bitcoin or USD $',
            ),
        ).toBe(true);
    });
    describe('Determining whether Send button should be disabled on SendXec screen', () => {
        const { expectedReturns } = vectors.shouldSendXecBeDisabled;

        // Successfully created targetOutputs
        expectedReturns.forEach(expectedReturn => {
            const {
                description,
                formData,
                balanceSats,
                apiError,
                sendAmountError,
                sendAddressError,
                multiSendAddressError,
                sendWithCashtabMsg,
                cashtabMsgError,
                sendWithOpReturnRaw,
                opReturnRawError,
                priceApiError,
                isOneToManyXECSend,
                sendDisabled,
            } = expectedReturn;
            it(`shouldSendXecBeDisabled: ${description}`, () => {
                expect(
                    shouldSendXecBeDisabled(
                        formData,
                        balanceSats,
                        apiError,
                        sendAmountError,
                        sendAddressError,
                        multiSendAddressError,
                        sendWithCashtabMsg,
                        cashtabMsgError,
                        sendWithOpReturnRaw,
                        opReturnRawError,
                        priceApiError,
                        isOneToManyXECSend,
                    ),
                ).toBe(sendDisabled);
            });
        });
    });
    describe('Parses user input address strings with parseAddressInput', () => {
        const { expectedReturns } = vectors.parseAddressInput;

        // Successfully created targetOutputs
        expectedReturns.forEach(expectedReturn => {
            const {
                description,
                addressInput,
                balanceSats,
                userLocale,
                parsedAddressInput,
            } = expectedReturn;
            it(`parseAddressInput: ${description}`, () => {
                expect(
                    parseAddressInput(addressInput, balanceSats, userLocale),
                ).toStrictEqual(parsedAddressInput);
            });
        });
    });
    describe('Returns true if a given input meets alias spec or expected error msg if it does not', () => {
        const { expectedReturns } = vectors.meetsAliasSpecInputCases;

        // Successfully created targetOutputs
        expectedReturns.forEach(expectedReturn => {
            const { description, inputStr, response } = expectedReturn;
            it(`meetsAliasSpec: ${description}`, () => {
                expect(meetsAliasSpec(inputStr)).toBe(response);
            });
        });
    });
    describe('Validates user alias input on Send and SendToken screens', () => {
        const { expectedReturns } = vectors.validAliasSendInputCases;

        // Successfully created targetOutputs
        expectedReturns.forEach(expectedReturn => {
            const { description, sendToAliasInput, response } = expectedReturn;
            it(`isValidAliasSendInput: ${description}`, () => {
                expect(isValidAliasSendInput(sendToAliasInput)).toBe(response);
            });
        });
    });
    describe('Validating Cashtab Contact Lists', () => {
        const { expectedReturns } = vectors.isValidContactList;
        expectedReturns.forEach(expectedReturn => {
            const { description, contactList, isValid } = expectedReturn;
            it(`isValidContactList: ${description}`, () => {
                expect(isValidContactList(contactList)).toBe(isValid);
            });
        });
    });
    describe('Appropriately migrates users with legacy settings', () => {
        const { expectedReturns } = vectors.migrateLegacyCashtabSettings;
        expectedReturns.forEach(expectedReturn => {
            const { description, legacySettings, migratedSettings } =
                expectedReturn;
            it(`migrateLegacyCashtabSettings: ${description}`, () => {
                expect(migrateLegacyCashtabSettings(legacySettings)).toEqual(
                    migratedSettings,
                );
            });
        });
    });
    describe('Determines if the user has valid cashtab settings', () => {
        const { expectedReturns } = vectors.isValidCashtabSettings;
        expectedReturns.forEach(expectedReturn => {
            const { description, settings, isValid } = expectedReturn;
            it(`isValidCashtabSettings: ${description}`, () => {
                expect(isValidCashtabSettings(settings)).toBe(isValid);
            });
        });
    });
    describe('Determines if cashtabCache is valid or invalid', () => {
        const { expectedReturns } = vectors.isValidCashtabCache;
        expectedReturns.forEach(expectedReturn => {
            const { description, cashtabCache, isValid } = expectedReturn;
            it(`isValidCashtabCache: ${description}`, () => {
                expect(isValidCashtabCache(cashtabCache)).toBe(isValid);
            });
        });
    });
    describe('Determines if a cashtab wallet is valid or invalid', () => {
        const { expectedReturns } = vectors.isValidCashtabWallet;
        expectedReturns.forEach(expectedReturn => {
            const { description, wallet, returned } = expectedReturn;
            it(`isValidCashtabWallet: ${description}`, () => {
                expect(isValidCashtabWallet(wallet)).toBe(returned);
            });
        });
    });
    describe('Determines if a user input send amount is valid', () => {
        const { expectedReturns } = vectors.isValidXecSendAmount;
        expectedReturns.forEach(expectedReturn => {
            const {
                description,
                sendAmount,
                balanceSats,
                userLocale,
                selectedCurrency,
                fiatPrice,
                returned,
            } = expectedReturn;
            it(`isValidXecSendAmount: ${description}`, () => {
                expect(
                    isValidXecSendAmount(
                        sendAmount,
                        balanceSats,
                        userLocale,
                        selectedCurrency,
                        fiatPrice,
                    ),
                ).toBe(returned);
            });
        });
    });
    describe('Determines if a user input multi-send CSV is valid', () => {
        const { expectedReturns } = vectors.isValidMultiSendUserInput;
        expectedReturns.forEach(expectedReturn => {
            const {
                description,
                userMultisendInput,
                balanceSats,
                userLocale,
                returned,
            } = expectedReturn;
            it(`isValidMultiSendUserInput: ${description}`, () => {
                expect(
                    isValidMultiSendUserInput(
                        userMultisendInput,
                        balanceSats,
                        userLocale,
                    ),
                ).toBe(returned);
            });
        });
    });
    describe('Determines if a user input send or burn token amount is valid', () => {
        const { expectedReturns } = vectors.isValidTokenSendOrBurnAmount;
        expectedReturns.forEach(expectedReturn => {
            const { description, amount, tokenBalance, decimals, returned } =
                expectedReturn;
            it(`isValidTokenSendOrBurnAmount: ${description}`, () => {
                expect(
                    isValidTokenSendOrBurnAmount(
                        amount,
                        tokenBalance,
                        decimals,
                    ),
                ).toBe(returned);
            });
        });
    });
    describe('Determines if a user input token mint amount is valid', () => {
        const { expectedReturns } = vectors.isValidTokenMintAmount;
        expectedReturns.forEach(expectedReturn => {
            const { description, amount, decimals, returned } = expectedReturn;
            it(`isValidTokenMintAmount: ${description}`, () => {
                expect(isValidTokenMintAmount(amount, decimals)).toBe(returned);
            });
        });
    });
    describe('Can tell if a string is valid bip21 op_return_raw input, or why it is not', () => {
        const { expectedReturns } = vectors.getOpReturnRawError;
        expectedReturns.forEach(expectedReturn => {
            const { description, opReturnRaw, returned } = expectedReturn;
            it(`getOpReturnRawError: ${description}`, () => {
                expect(getOpReturnRawError(opReturnRaw)).toBe(returned);
            });
        });
    });
    describe('Validates OP_RETURN raw for eCash node broadcast', () => {
        const { expectedReturns } = vectors.nodeWillAcceptOpReturnRaw;
        expectedReturns.forEach(expectedReturn => {
            const { description, opReturnRaw, returned } = expectedReturn;
            it(`nodeWillAcceptOpReturnRaw: ${description}`, () => {
                expect(nodeWillAcceptOpReturnRaw(opReturnRaw)).toBe(returned);
            });
        });
    });
});
