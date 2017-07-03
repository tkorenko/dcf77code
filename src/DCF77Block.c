#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include "DCF77Block.h"

#define LOCBUF_SZ 4

void
DCF77Block_FromText(const char * textSrc,
	DCF77Block_t * pBinDst)
{
	char locBuf[LOCBUF_SZ];
	int i;

	if (NULL == textSrc || NULL == pBinDst)
		return;

	if (strlen(textSrc) < DCF77BLOCK_TEXT_LEN) {
		errx(EX_DATAERR, "invalid text length");
		/* NOTREACHED */
	}

	for (i = 0; i < DCF77BLOCK_SIZE; ++i) {
		int j = 2 * i;

		locBuf[0] = textSrc[j];
		locBuf[1] = textSrc[j + 1];
		locBuf[2] = '\0';

		pBinDst->data[i] = (uint8_t)strtol(locBuf, NULL, 16);
	}
}

void
DCF77Block_ToText(const DCF77Block_t * pBinSrc,
	char * textDst, size_t textDstSz)
{
	char locBuf[LOCBUF_SZ];
	int i;

	if (NULL == pBinSrc || NULL == textDst)
		return;

	if (textDstSz < (DCF77BLOCK_TEXT_LEN + 1)) {
		errx(EX_DATAERR, "insufficient space for output");
		/* NOTREACHED */
	}

	for (i = 0; i < DCF77BLOCK_SIZE; ++i) {
		uint8_t byte = pBinSrc->data[i];

		snprintf(locBuf, LOCBUF_SZ, "%02hhX", byte);
		*textDst = locBuf[0];
		++textDst;
		*textDst = locBuf[1];
		++textDst;
	}
	*textDst = '\0';
}
