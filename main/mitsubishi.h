#ifndef SONY_H
#define SONY_H

#include <stdint.h>
#include "driver/rmt_tx.h"

#define MITSUBISHI_ACTION_POWER 0

typedef uint16_t mitsubishi_action_t;
typedef struct mitsubishi_frame_rmt_symbols {
  rmt_symbol_word_t symbols[292];
} mitsubishi_frame_rmt_symbols_t;

/**
 * @param action action to encode
 * @param isLast 1 if action won't be repeated anymore, 0 otherwise
 * @return Output RMT symbols wrapper containing a constant number of 292 symbols representing the input action
 */
mitsubishi_frame_rmt_symbols_t mitsubishi_encode_action(mitsubishi_action_t action, int isLast);

/**
 * @param rmtSymbolsIn RMT symbols to repeat in a way such that Mitsubishi is happy
 * @param pRmtSymbolsOut pointer to target memory area where we should store the repeated RMT symbols. Shoud have at least sizeof(sony_frame_rmt_symbols_t) * n bytes available
 * @param n number of times the frame's RMT symbols will be inserted into the output RMT symbols sequence
 */
void mitsubishi_repeat(mitsubishi_action_rmt_symbols_t rmtSymbolsIn, rmt_symbol_word_t* pRmtSymbolsOut, int n);

#endif //SONY_H