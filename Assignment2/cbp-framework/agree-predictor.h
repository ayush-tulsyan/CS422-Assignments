/* Author: Jared Stark;   Created: Tue Jul 27 13:39:15 PDT 2004
 * Description: This file defines a gshare branch predictor.
 */

#ifndef PREDICTOR_H_SEEN
#define PREDICTOR_H_SEEN

#include <cstddef>
#include <inttypes.h>
#include <vector>
#include "op_state.h"   // defines op_state_c (architectural state) class
#include "tread.h"      // defines branch_record_c class

class PREDICTOR
{
public:
    typedef uint32_t address_t;

private:
    typedef uint32_t history_t;
    typedef uint8_t counter_t;

    static const int BHR_LENGTH = 14, BST_WIDTH = 14, BST_INIT_WIDTH = 14;
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PHT_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT_INDEX_MASK = (PHT_SIZE - 1);
    static const std::size_t BST_SIZE = (std::size_t(1) << BST_WIDTH);
    static const std::size_t BST_INDEX_MASK = (BST_SIZE - 1);
    static const counter_t PHT_INIT = /* strongly not taken */ 2;
    static const counter_t BST_INIT = 0;

    history_t bhr;                      // 13 bits
    std::vector<counter_t> pht;         // 32K bits
    std::vector<counter_t> bst;         // 16K bits
    std::vector<counter_t> bst_init;    // 16K bits

    void update_bhr(bool taken) { bhr >>= 1; if (taken) bhr |= BHR_MSB; }
    static std::size_t pht_index(address_t pc, history_t bhr)
    { return (static_cast<std::size_t>(pc ^ bhr) & PHT_INDEX_MASK); }
    static std::size_t bst_index(address_t pc)
    { return (static_cast<std::size_t> (pc) & BST_INDEX_MASK); }

    static bool counter_msb(/* 2-bit counter */ counter_t cnt) { return (cnt >> 1); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 3) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 0) --cnt; return cnt; }

public:
    PREDICTOR(void) : bhr(0), pht(PHT_SIZE, counter_t(PHT_INIT)),
    bst(BST_SIZE, counter_t(BST_INIT)), bst_init(BST_SIZE, counter_t(0))
    { }
    // uses compiler generated copy constructor
    // uses compiler generated destructor
    // uses compiler generated assignment operator

    // get_prediction() takes a branch record (br, branch_record_c is defined in
    // tread.h) and architectural state (os, op_state_c is defined op_state.h).
    // Your predictor should use this information to figure out what prediction it
    // wants to make.  Keep in mind you're only obligated to make predictions for
    // conditional branches.
    bool get_prediction(const branch_record_c* br, const op_state_c* os)
    {
        bool prediction = false;
        if (/* conditional branch */ br->is_conditional) {
            address_t pc = br->instruction_addr;
            std::size_t pht_idx = pht_index(pc, bhr);
            std::size_t bst_idx = bst_index(pc);
            if ( bst_init[bst_idx] == 0 ) {
                prediction = counter_msb(pht[pht_idx]);
            }
            else {
                counter_t bias = bst[bst_idx];
                counter_t pht_pred = pht[pht_idx];
                if ( bias == counter_msb(pht_pred) ) prediction = true;
                else prediction = false;
            }
        }
        return prediction;   // true for taken, false for not taken
    }

    // Update the predictor after a prediction has been made.  This should accept
    // the branch record (br) and architectural state (os), as well as a third
    // argument (taken) indicating whether or not the branch was taken.
    void update_predictor(const branch_record_c* br, const op_state_c* os, bool taken)
    {
        if (/* conditional branch */ br->is_conditional) {
            address_t pc = br->instruction_addr;
            std::size_t pht_idx = pht_index(pc, bhr);
            std::size_t bst_idx = bst_index(pc);
            // [TODO]: To set the bias bit optimally
            if ( bst_init[bst_idx] == 0 ) {
                bst_init[bst_idx] = 1;
                bst[bst_idx] = taken;
            }
            else {
                counter_t bias = bst[bst_idx];
                counter_t pht_pred = pht[pht_idx];
                if ( bias == taken )
                    pht_pred = counter_inc(pht_pred);
                else
                    pht_pred = counter_dec(pht_pred);
                pht[pht_idx] = pht_pred;
            }
            update_bhr(taken);
        }
    }
};

#endif // PREDICTOR_H_SEEN

