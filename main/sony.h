#ifndef SONY_H
#define SONY_H

#include <stdint.h>
#include "driver/rmt_tx.h"

#define SONY_FRAME_POWER 0x095
#define SONY_FRAME_1 0x080
#define SONY_FRAME_2 0x081
#define SONY_FRAME_3 0x082
#define SONY_FRAME_4 0x083
#define SONY_FRAME_5 0x084
#define SONY_FRAME_6 0x085
#define SONY_FRAME_7 0x086
#define SONY_FRAME_8 0x087
#define SONY_FRAME_9 0x088
#define SONY_FRAME_0 0x089
#define SONY_FRAME_EXIT 0x0E3
#define SONY_FRAME_TT 0x0BF
#define SONY_FRAME_PROG_NEXT 0x090
#define SONY_FRAME_PROG_PREV 0x091
#define SONY_FRAME_VOLUP 0x092
#define SONY_FRAME_VOLDN 0x093

typedef uint16_t sony_frame_t;
typedef struct sony_frame_rmt_symbols {
  rmt_symbol_word_t symbols[13];
} sony_frame_rmt_symbols_t;

/**
 * @param frame frame to encode
 * @param isLast 1 if frame won't be repeated anymore, 0 otherwise
 * @return Output RMT symbols wrapper containing a constant number of 13 symbols representing the input frame
 */
sony_frame_rmt_symbols_t sony_encode_frame(sony_frame_t frame, int isLast);

/**
 * @param rmtSymbolsIn RMT symbols to repeat in a way such that Sony is happy
 * @param pRmtSymbolsOut pointer to target memory area where we should store the repeated RMT symbols. Shoud have at least sizeof(sony_frame_rmt_symbols_t) * n bytes available
 * @param n number of times the frame's RMT symbols will be inserted into the output RMT symbols sequence
 */
void sony_repeat(sony_frame_rmt_symbols_t rmtSymbolsIn, rmt_symbol_word_t* pRmtSymbolsOut, int n);

#endif //SONY_H