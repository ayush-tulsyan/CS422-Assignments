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

    static const int BHR_LENGTH = 15;
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PHT_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT_INDEX_MASK = (PHT_SIZE - 1);
    static const counter_t PHT_INIT = /* weakly taken */ 2;

    history_t bhr;                // 15 bits
    std::vector<counter_t> pht;   // 64K bits

    void update_bhr(bool taken) { bhr >>= 1; if (taken) bhr |= BHR_MSB; }
    static std::size_t pht_index(address_t pc, history_t bhr)
    { return (static_cast<std::size_t>(pc ^ bhr) & PHT_INDEX_MASK); }
    static bool counter_msb(/* 2-bit counter */ counter_t cnt) { return (cnt >= 2); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 3) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 0) --cnt; return cnt; }

public:
    PREDICTOR(void) : bhr(0), pht(PHT_SIZE, counter_t(PHT_INIT)) { }
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
            std::size_t index = pht_index(pc, bhr);
            counter_t cnt = pht[index];
            prediction = counter_msb(cnt);
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
            std::size_t index = pht_index(pc, bhr);
            counter_t cnt = pht[index];
            if (taken)
                cnt = counter_inc(cnt);
            else
                cnt = counter_dec(cnt);
            pht[index] = cnt;
            update_bhr(taken);
        }
    }
};

#endif // PREDICTOR_H_SEEN

