// Copyright (c) 2023 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

'use strict';
const config = require('../config');
const log = require('./log');
const { ChronikClient } = require('chronik-client');
const chronik = new ChronikClient(config.chronik);

module.exports = {
    chronik,
    getTxHistoryPage: async function (hash160, page = 0) {
        let txHistoryPage;
        try {
            txHistoryPage = await chronik
                .script('p2pkh', hash160)
                // Get the 25 most recent transactions
                .history(page, config.txHistoryPageSize);
            return txHistoryPage;
        } catch (err) {
            log(`Error in getTxHistoryPage(${hash160})`, err);
        }
    },
    returnGetTxHistoryPagePromise: async function (hash160, page = 0) {
        /* 
        Unlike getTxHistoryPage, this function will reject and 
        fail Promise.all() if there is an error in the chronik call
        */
        return new Promise((resolve, reject) => {
            chronik
                .script('p2pkh', hash160)
                .history(page, config.txHistoryPageSize)
                .then(
                    result => {
                        resolve(result);
                    },
                    err => {
                        reject(err);
                    },
                );
        });
    },
    getUnprocessedTxHistory: async function (
        hash160,
        processedBlockheight,
        processedTxCount,
        optionalMocks = false,
    ) {
        let potentiallyUnprocessedTxs = [];

        // optionalMocks is a param used to inject mock chronik data for unit tests
        /*
        optionalMocks = {txHistoryFirstPageResponse, remainingTxHistoryPageResponses}
        */

        // Get first page of most recent chronik tx history
        const txHistoryFirstPageResponse = optionalMocks
            ? optionalMocks.txHistoryFirstPageResponse
            : await module.exports.getTxHistoryPage(hash160);
        const { txs, numPages } = txHistoryFirstPageResponse;

        // This first page of results contains the most recent chronik txs at the address
        // These are potentially unprocessed, needs to be checked
        potentiallyUnprocessedTxs = potentiallyUnprocessedTxs.concat(txs);

        // Do you have enough history with just the one page?
        let alreadyHaveAllPotentiallyUnprocessedTxs = false;
        const oldestTxOnFirstPage = txs[txs.length - 1];
        // If the oldest tx on the first page is unconfirmed, you need more pages
        if (typeof oldestTxOnFirstPage.block === 'undefined') {
            alreadyHaveAllPotentiallyUnprocessedTxs = false;
        } else {
            // If the oldest tx on the first page hasa blockheight that has already been processed
            if (oldestTxOnFirstPage.block.height <= processedBlockheight) {
                // Then you have enough txs with this one call
                alreadyHaveAllPotentiallyUnprocessedTxs = true;
            }
        }
        // If you may need more txs
        let maxTxs, maxUnprocessedTxCount, numPagesToFetch;
        if (!alreadyHaveAllPotentiallyUnprocessedTxs) {
            // Determine maximum number of txs at the alias registration address
            maxTxs = config.txHistoryPageSize * numPages;
            maxUnprocessedTxCount = maxTxs - processedTxCount;
            numPagesToFetch = Math.ceil(
                maxUnprocessedTxCount / config.txHistoryPageSize,
            );

            // Iterate through numPagesToFetch to get all potentiallyUnprocessedTxs
            // Start with i=1, as you already have data from page 0
            // Note: Since 0 is a page number, 3 pages of data ends with pageNumber i=2
            const txHistoryPageResponsePromises = [];
            for (let i = 1; i < numPagesToFetch; i += 1) {
                const txHistoryPageResponsePromise =
                    module.exports.returnGetTxHistoryPagePromise(hash160, i);
                txHistoryPageResponsePromises.push(
                    txHistoryPageResponsePromise,
                );
            }
            // Use Promise.all so that an error is thrown if any single promise fails
            let remainingTxHistoryPageResponses;
            try {
                remainingTxHistoryPageResponses = optionalMocks
                    ? optionalMocks.remainingTxHistoryPageResponses
                    : await Promise.all(txHistoryPageResponsePromises);
            } catch (err) {
                log(`Error in Promise.all(txHistoryPageResponsePromises)`, err);
                // Return false; you won't have all the tx history if this happens
                return false;
            }
            // add the rest of these potentially unprocessed txs to potentiallyUnprocessedTxs
            for (
                let i = 0;
                i < remainingTxHistoryPageResponses.length;
                i += 1
            ) {
                const { txs } = remainingTxHistoryPageResponses[i];
                potentiallyUnprocessedTxs =
                    potentiallyUnprocessedTxs.concat(txs);
            }
        }

        // Iterate over potentiallyUnprocessedTxs, adding everything you haven't processed
        const unprocessedTxs = [];
        for (let i = 0; i < potentiallyUnprocessedTxs.length; i += 1) {
            const potentiallyUnprocessedTx = potentiallyUnprocessedTxs[i];
            // unconfirmed txs are unprocessed
            if (typeof potentiallyUnprocessedTx.block === 'undefined') {
                unprocessedTxs.push(potentiallyUnprocessedTx);
            } else {
                if (
                    potentiallyUnprocessedTx.block.height > processedBlockheight
                ) {
                    unprocessedTxs.push(potentiallyUnprocessedTx);
                }
            }
        }
        if (optionalMocks) {
            // Return spy variables to make sure they check out in unit tests
            return {
                maxTxs,
                maxUnprocessedTxCount,
                numPagesToFetch,
                alreadyHaveAllPotentiallyUnprocessedTxs,
                unprocessedTxs,
            };
        } else {
            return unprocessedTxs;
        }
    },
    getAllTxHistory: async function (hash160) {
        let allTxHistory = [];
        const txHistoryFirstPageResponse =
            await module.exports.getTxHistoryPage(hash160);
        const { txs, numPages } = txHistoryFirstPageResponse;

        // Add first page of results to allTxHistory
        allTxHistory = allTxHistory.concat(txs);

        // Iterate through remaining pages to get remaining tx history
        // Start with i=1, as you already have data from page 0
        // Note: Since 0 is a page number, 3 pages of data ends with pageNumber i=2
        const txHistoryPageResponsePromises = [];
        for (let i = 1; i < numPages; i += 1) {
            const txHistoryPageResponsePromise =
                module.exports.returnGetTxHistoryPagePromise(hash160, i);
            txHistoryPageResponsePromises.push(txHistoryPageResponsePromise);
        }

        // Use Promise.all so that an error is thrown if any single promise fails
        let remainingTxHistoryPageResponses;
        try {
            remainingTxHistoryPageResponses = await Promise.all(
                txHistoryPageResponsePromises,
            );
        } catch (err) {
            log(`Error in Promise.all(txHistoryPageResponsePromises)`, err);
            // Return false; you won't have all the tx history if this happens
            return false;
        }

        // Iterate over results to complete allTxHistory
        for (let i = 0; i < remainingTxHistoryPageResponses.length; i += 1) {
            const { txs } = remainingTxHistoryPageResponses[i];
            allTxHistory = allTxHistory.concat(txs);
        }

        return allTxHistory;
    },
};
