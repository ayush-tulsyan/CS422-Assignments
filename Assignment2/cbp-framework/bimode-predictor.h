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

    static const int BHR_LENGTH = 13, CPT_WIDTH = 14;
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PHT_SIZE = (std::size_t(1) << BHR_LENGTH);
    static const std::size_t PHT_INDEX_MASK = (PHT_SIZE - 1);
    static const std::size_t CPT_SIZE = (std::size_t(1) << CPT_WIDTH);
    static const std::size_t CPT_INDEX_MASK = (CPT_SIZE - 1);
    static const counter_t PHT_INIT_0 = /* strongly not taken */ 0;
    static const counter_t PHT_INIT_1 = /* strongly taken */ 3;
    static const counter_t CPT_INIT = /* weakly towards 0 */ 1;

    history_t bhr;                      // 13 bits
    std::vector<counter_t> pht[2];      // 32K bits
    std::vector<counter_t> cpt;         // 32K bits

    void update_bhr(bool taken) { bhr >>= 1; if (taken) bhr |= BHR_MSB; }
    static std::size_t pht_index(address_t pc, history_t bhr)
    { return (static_cast<std::size_t>(pc ^ bhr) & PHT_INDEX_MASK); }
    static std::size_t cpt_index(address_t pc)
    { return (static_cast<std::size_t> (pc) & CPT_INDEX_MASK ); }

    static bool counter_msb(/* 2-bit counter */ counter_t cnt) { return (cnt >> 1); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 3) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
    { if (cnt != 0) --cnt; return cnt; }

public:
    PREDICTOR(void) : bhr(0), pht{ {std::vector <counter_t>(PHT_SIZE, counter_t(PHT_INIT_0)) },
        {std::vector <counter_t>(PHT_SIZE, counter_t(PHT_INIT_1)) } },
    cpt(CPT_SIZE, counter_t(CPT_INIT)) { }
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
            bool choice = counter_msb(cpt[cpt_index(pc)]);
            counter_t cnt = pht[choice][pht_idx];
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

            std::size_t pht_idx = pht_index(pc, bhr);
            std::size_t cpt_idx = cpt_index(pc);
            counter_t choice_cnt = cpt[cpt_idx];
            bool choice = counter_msb(choice_cnt);
            counter_t pht_cnt = pht[choice][pht_idx];
            counter_t pht_cnt_updated;

            if (taken)
                pht_cnt_updated = counter_inc(pht_cnt);
            else
                pht_cnt_updated = counter_dec(pht_cnt);
            pht[choice][pht_idx] = pht_cnt_updated;

            if (counter_msb(pht_cnt) == counter_msb(pht_cnt_updated)) {
                if (counter_msb(pht_cnt_updated) == taken)
                    choice_cnt = (choice==1)?counter_inc(choice_cnt):counter_dec(choice_cnt);
                else
                    choice_cnt = (choice==1)?counter_dec(choice_cnt):counter_inc(choice_cnt);
            }
            cpt[cpt_idx] = choice_cnt;

            update_bhr(taken);
        }
    }
};

#endif // PREDICTOR_H_SEEN

