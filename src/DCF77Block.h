#ifndef D_DCF77Block_h
#define D_DCF77Block_h

#include <stdint.h>

enum {
	DCF77BLOCK_SIZE = 8,
	DCF77BLOCK_TEXT_LEN = 16
};

typedef struct {
	uint8_t data[DCF77BLOCK_SIZE];
} DCF77Block_t;

void DCF77Block_FromText(const char * textSrc, DCF77Block_t * pBinDst);
void DCF77Block_ToText(const DCF77Block_t * pBinSrc,
	char * textDst, size_t textDstSz);

#endif /* #ifndef D_DCF77Block_h */
