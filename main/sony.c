#include "sony.h"

rmt_symbol_word_t sync = {
  .duration0 = 2454,
  .level0 = 1,
  .duration1 = 585,
  .level1 = 0
};

rmt_symbol_word_t one = {
  .duration0 = 1223,
  .level0 = 1,
  .duration1 = 586,
  .level1 = 0
};

rmt_symbol_word_t zero = {
  .duration0 = 617,
  .level0 = 1,
  .duration1 = 585,
  .level1 = 0
};

static void sony_terminate_rmt_symbol(rmt_symbol_word_t* pRmtSymbol, int isLastFrame) {
  pRmtSymbol->duration1 = isLastFrame ? 0 : 25855;
  //pRmtSymbol->level1 = isLastFrame ? 1 : 0;
}

sony_frame_rmt_symbols_t sony_encode_frame(sony_frame_t frame, int isLast) {
  sony_frame_rmt_symbols_t rmtSymbols = {0};
  rmtSymbols.symbols[0] = sync;
  
  // pRmtSymbols[1] = (frame & (uint16_t)1) ? one : zero;
  // pRmtSymbols[2] = (frame & ((uint16_t)1 << 1)) ? one : zero;
  
  for (int i = 0; i < 12; i++) {
    rmtSymbols.symbols[i + 1] = (frame & (((uint16_t)1) << i)) ? one : zero;
  }

  sony_terminate_rmt_symbol(&rmtSymbols.symbols[12], isLast);
  return rmtSymbols;
}

void sony_repeat(sony_frame_rmt_symbols_t rmtSymbolsIn, rmt_symbol_word_t* pRmtSymbolsOut, int n) {
  const int s = sizeof(rmtSymbolsIn.symbols) / sizeof(rmt_symbol_word_t);
  assert(s == 13);
  if (pRmtSymbolsOut && (n > 0)) {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < s; j++) {
        pRmtSymbolsOut[i * s + j] = rmtSymbolsIn.symbols[j];
      }
      sony_terminate_rmt_symbol(&pRmtSymbolsOut[i * s + s - 1], 0);
    }
    sony_terminate_rmt_symbol(&pRmtSymbolsOut[(n - 1) * s + s - 1], 1);
  }
}