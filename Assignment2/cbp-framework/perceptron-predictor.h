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
    typedef uint64_t history_t;
    typedef int16_t counter_t;

    static const int BHR_LENGTH = 31, PT_INDEX_LENGTH = 8;
    static const int PT_BITS_PER_WEIGHT = 8, PT_NUM_WEIGHTS = BHR_LENGTH + 1;
    static const history_t BHR_MSB = (history_t(1) << (BHR_LENGTH - 1));
    static const std::size_t PT_SIZE = (std::size_t(1) << PT_INDEX_LENGTH);
    static const std::size_t PT_INDEX_MASK = (PT_SIZE - 1);
    static const counter_t PT_INIT = 0;
    static const counter_t PT_WEIGHT_LIMIT =
        (std::size_t(1) << (PT_BITS_PER_WEIGHT-1)) - 1;

    history_t bhr;                      // 31 bits
    std::vector<counter_t> pt[PT_SIZE];      // 64K bits

    void update_bhr(bool taken) { bhr >>= 1; if (taken) bhr |= BHR_MSB; }
    static std::size_t pt_index(address_t pc)
    { return (static_cast<std::size_t>(pc) & PT_INDEX_MASK); }

    static bool counter_msb(/* Perceptron output */ counter_t y)
    { return (y >= 0); }
    static counter_t counter_inc(/* 2-bit counter */ counter_t cnt)
    { if (cnt != PT_WEIGHT_LIMIT) ++cnt; return cnt; }
    static counter_t counter_dec(/* 2-bit counter */ counter_t cnt)
    { if (cnt != -PT_WEIGHT_LIMIT) --cnt; return cnt; }

public:
    PREDICTOR(void) {
        bhr = 0;
        for(std::size_t i = 0; i < PT_SIZE; ++i )
            pt[i] = std::vector <counter_t>(PT_NUM_WEIGHTS, counter_t(PT_INIT));
    }
    // uses compiler generated copy constructor
    // uses compiler generated destructor
    // uses compiler generated assignment operator

    // get_perceptron_output() takes the perceptron index, uses branch history
    // register and calculates the dot product of weight vector stored in
    // Perceptron table and history register.
    counter_t get_perceptron_output(std::size_t pt_idx) {
        counter_t y = pt[pt_idx][0];
        history_t bhr_copy = bhr;

        for(int i = 1; i <= BHR_LENGTH; ++i, bhr_copy <<= 1) {
            if (bhr_copy & BHR_MSB)
                y += pt[pt_idx][i];
            else
                y -= pt[pt_idx][i];
        }
        return y;
    }

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
            std::size_t pt_idx = pt_index(pc);
            counter_t y = get_perceptron_output(pt_idx);

            prediction = counter_msb(y);
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
            std::size_t pt_idx = pt_index(pc);
            counter_t y = get_perceptron_output(pt_idx);

            bool prediction = counter_msb(y);

            counter_t y_abs = y*((prediction)?1:-1);

            if ((prediction != taken) & (y_abs <= PT_WEIGHT_LIMIT)) {
                counter_t t = (taken) ? 1 : -1;
                address_t bhr_copy = bhr;

                pt[pt_idx][0] += t;
                for (std::size_t i = 1; i <= BHR_LENGTH; ++i, bhr_copy <<= 1) {
                    if ((bhr_copy & BHR_MSB) == taken)
                        pt[pt_idx][i] += 1;
                    else
                        pt[pt_idx][i] -= 1;
                }
            }

            update_bhr(taken);
        }
    }
};

#endif // PREDICTOR_H_SEEN

