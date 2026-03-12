#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct in_rmt_symbol {
  uint16_t duration0;
  uint16_t level0;
  uint16_t duration1;
  uint16_t level1;
} in_rmt_symbol_t;

typedef enum in_symbol_type {
  IN_SYMBOL_TYPE_START,
  IN_SYMBOL_TYPE_SHORT,
  IN_SYMBOL_TYPE_LONG,
  IN_SYMBOL_TYPE_GAP,
  IN_SYMBOL_TYPE_TERM,
} in_symbol_type_t;

int in_parse_rmt_line(const char* line, in_rmt_symbol_t* pSymbol_out) {
  assert(line != NULL);
  in_rmt_symbol_t symbol;
  memset(&symbol, 0, sizeof(symbol));
  int n = sscanf(line, "\x1b[0;32mI (%*d) ies0: symbol[%*d]: duration0=%hu, level0=%hu, duration1=%hu, level1=%hu\x1b[0m\r", &symbol.duration0, &symbol.level0, &symbol.duration1, &symbol.level1);
  //printf("[%d]", n);
  if (4 == n) {
    if (pSymbol_out) {
      *pSymbol_out = (in_rmt_symbol_t)symbol;
    }
    return 1;
  } else {
    return 0;
  }
}

int in_parse_rmt_start_line(const char* line) {
  assert(line != NULL);
  int pos = 0;
  int n = sscanf(line, "\x1b[0;32mI (%*d) ies0: Received RMT data, num_symbols=%*d, is_last=%*d\x1b[0m\r%n", &pos);
  if (n == 0 && pos > 0) {
    return 1;
  } else {
    return 0;
  }
}

#define DETECTOR_START_LVL0_DUR 3500
#define DETECTOR_START_LVL1_DUR 1700
#define DETECTOR_START_LVL0_TOLERANCE 200
#define DETECTOR_START_LVL1_TOLERANCE 200

#define DETECTOR_SHORT_LVL0_DUR 450
#define DETECTOR_SHORT_LVL1_DUR 400
#define DETECTOR_SHORT_LVL0_TOLERANCE 100
#define DETECTOR_SHORT_LVL1_TOLERANCE 100

#define DETECTOR_LONG_LVL0_DUR 450
#define DETECTOR_LONG_LVL1_DUR 1250
#define DETECTOR_LONG_LVL0_TOLERANCE 100
#define DETECTOR_LONG_LVL1_TOLERANCE 200

#define DETECTOR_GAP_LVL0_DUR 450
#define DETECTOR_GAP_LVL1_DUR 17000
#define DETECTOR_GAP_LVL0_TOLERANCE 100
#define DETECTOR_GAP_LVL1_TOLERANCE 500

#define DETECTOR_TERM_LVL0_DUR 465
#define DETECTOR_TERM_LVL1_DUR 0
#define DETECTOR_TERM_LVL0_TOLERANCE 100
#define DETECTOR_TERM_LVL1_TOLERANCE 0

int detect_x(in_rmt_symbol_t symbol, uint16_t xLvl0Dur, uint16_t xLvl1Dur, uint16_t xLvl0Tol, uint16_t xLvl1Tol) {
  if (!(symbol.level0 == 0 && symbol.level1 == 1)) {
    return 0;
  }
  if ((symbol.duration0 >= xLvl0Dur - xLvl0Tol) 
      && (symbol.duration0 <= xLvl0Dur + xLvl0Tol)
      && (symbol.duration1 >= xLvl1Dur - xLvl1Tol)
      && (symbol.duration1 <= xLvl1Dur + xLvl1Tol)
  ) {
    return 1; 
  }
  return 0;
}

int detect_start(in_rmt_symbol_t symbol) {
  return detect_x(symbol, DETECTOR_START_LVL0_DUR, DETECTOR_START_LVL1_DUR, DETECTOR_START_LVL0_TOLERANCE, DETECTOR_START_LVL1_TOLERANCE);
}

int detect_short(in_rmt_symbol_t symbol) { 
  return detect_x(symbol, DETECTOR_SHORT_LVL0_DUR, DETECTOR_SHORT_LVL1_DUR, DETECTOR_SHORT_LVL0_TOLERANCE, DETECTOR_SHORT_LVL1_TOLERANCE);
}

int detect_long(in_rmt_symbol_t symbol) {
  return detect_x(symbol, DETECTOR_LONG_LVL0_DUR, DETECTOR_LONG_LVL1_DUR, DETECTOR_LONG_LVL0_TOLERANCE, DETECTOR_LONG_LVL1_TOLERANCE);
}

int detect_gap(in_rmt_symbol_t symbol) {
  return detect_x(symbol, DETECTOR_GAP_LVL0_DUR, DETECTOR_GAP_LVL1_DUR, DETECTOR_GAP_LVL0_TOLERANCE, DETECTOR_GAP_LVL1_TOLERANCE); 
}

int detect_term(in_rmt_symbol_t symbol) {
  return detect_x(symbol, DETECTOR_TERM_LVL0_DUR, DETECTOR_TERM_LVL1_DUR, DETECTOR_TERM_LVL0_TOLERANCE, DETECTOR_TERM_LVL1_TOLERANCE);  
}

int obtain_symbol_type(in_rmt_symbol_t symbol, in_symbol_type_t* pType_out) {
  if (detect_short(symbol)) {
    if (pType_out) {
      *pType_out = IN_SYMBOL_TYPE_SHORT;
    }
    return 1;
  } else if (detect_long(symbol)) {
    if (pType_out) {
      *pType_out = IN_SYMBOL_TYPE_LONG;
    }
    return 1;
  } else if (detect_start(symbol)) {
    if (pType_out) {
      *pType_out = IN_SYMBOL_TYPE_START;
    }
    return 1;
  } else if (detect_gap(symbol)) {
    if (pType_out) {
      *pType_out = IN_SYMBOL_TYPE_GAP;
    }
    return 1;
  } else if (detect_term(symbol)) {
    if (pType_out) {
      *pType_out = IN_SYMBOL_TYPE_TERM;
    }
  } else {
    return 0;
  }
}

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IOLBF, 0); //line-buffered stdout
  setvbuf(stdin, NULL, _IOLBF, 0); //line-buffered stdin				    
				   
  char line[512];
  in_rmt_symbol_t symbol;
  in_symbol_type_t symbol_type;
  while (fgets(line, sizeof(line), stdin)) {
    line[strcspn(line, "\n")] = 0; //Remove newline
    if (in_parse_rmt_line(line, &symbol)) {
      if (obtain_symbol_type(symbol, &symbol_type)) {
        switch (symbol_type) {
          case IN_SYMBOL_TYPE_SHORT:
            printf("0");
	    break;
	  case IN_SYMBOL_TYPE_LONG:
	    printf("1");
	    break;
	  case IN_SYMBOL_TYPE_START:
	    printf("S");
	    break;
	  case IN_SYMBOL_TYPE_GAP:
	    printf("-\n");
	    break;
	  case IN_SYMBOL_TYPE_TERM:
	    printf("t");
	    break;
	  default:
	    assert(0);
	    break;
	}
      } else {
        printf("x");
      }
    } else if (in_parse_rmt_start_line(line)) {
      printf("\n==================\n");
    }
  }

  printf("\n#####################\n");
  return 0;
}
