#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "DCF77Block.h"
#include "DCF77TimeCode.h"
#include "DCF77TimeCodePrivate.h"
#include "utils.h"

static void timeCode_Init(union TimeCodeConversion_t * pTcc);
static void timeCode_CopyIn(union TimeCodeConversion_t * pTcc,
	const DCF77Block_t * pInBlock);
static void timeCode_CopyOut(const union TimeCodeConversion_t * pTcc,
	DCF77Block_t * pOutBlock);
static unsigned int convertTwoDigitBCDtoInt(unsigned int v);
static unsigned int convertIntToTwoDigitBCD(unsigned int v);
static unsigned int computeEvenParityBit(unsigned int v);
static void breakBlockToTimecodeFields(const DCF77Block_t * pBlock);
static void splitBlockToStringOfBits(const DCF77Block_t * pBlock,
	char outBits[]);
static void regroupBitsIntoTimecodeFields(const char blockBits[]);


void
DCF77TimeCode_Init(DCF77Block_t * pBlock)
{
	union TimeCodeConversion_t tcc;

	if (NULL == pBlock)
		return;

	timeCode_Init(&tcc);
	timeCode_CopyOut(&tcc, pBlock);
}

static void
timeCode_Init(union TimeCodeConversion_t * pTcc)
{
	unsigned i;

	for (i = 0; i < DCF77BLOCK_SIZE; ++i) {
		pTcc->block.data[i] = 0u;
	}

	pTcc->dcfTc.S  = 1;
}

static void
timeCode_CopyOut(const union TimeCodeConversion_t * pTcc,
	DCF77Block_t * pOutBlock)
{
	unsigned i;

	for (i = 0; i < DCF77BLOCK_SIZE; ++i) {
		pOutBlock->data[i] = pTcc->block.data[i];
	}
}

void
DCF77TimeCode_ConvertToStructTM(const DCF77Block_t * pBlock,
	struct tm * outStm)
{
	union TimeCodeConversion_t tcc;

	if (NULL == pBlock || NULL == outStm)
		return;

	timeCode_CopyIn(&tcc, pBlock);

        memset(outStm, 0, sizeof(struct tm));
        outStm->tm_min   = convertTwoDigitBCDtoInt( tcc.dcfTc.minute );
        outStm->tm_hour  = convertTwoDigitBCDtoInt( tcc.dcfTc.hour );
        outStm->tm_mday  = convertTwoDigitBCDtoInt( tcc.dcfTc.dayOfMonth );
        outStm->tm_mon   = convertTwoDigitBCDtoInt( tcc.dcfTc.month ) - 1;
        outStm->tm_year  = convertTwoDigitBCDtoInt( tcc.dcfTc.year ) + 100;
        outStm->tm_wday  = (7 == tcc.dcfTc.dayOfWeek) ?
		0 : tcc.dcfTc.dayOfWeek;
        outStm->tm_isdst = tcc.dcfTc.Z1;
}

static void
timeCode_CopyIn(union TimeCodeConversion_t * pTcc,
	const DCF77Block_t * pInBlock)
{
	unsigned i;

	for (i = 0; i < DCF77BLOCK_SIZE; ++i) {
		pTcc->block.data[i] = pInBlock->data[i];
	}
}

static unsigned int
convertTwoDigitBCDtoInt(unsigned int v)
{
        unsigned int hi = (v & 0xF0u) >> 4;
        unsigned int lo =  v & 0x0Fu;

        return (10 * hi + lo);
}

static int timeCode_DSTChangeApproaching(const struct tm * inStm);

/*
 * We assume that our input (struct tm) has sane values in its fields.
 */
void
DCF77TimeCode_ConvertFromStructTM(DCF77Block_t * pBlock,
	const struct tm * inStm)
{
	union TimeCodeConversion_t tcc;

	if (NULL == pBlock || NULL == inStm)
		return;

	timeCode_Init(&tcc);

	int mon  = inStm->tm_mon + 1;
	int wday = inStm->tm_wday;

	wday = (0 == wday) ? 7 : wday;

	tcc.dcfTc.A1 = timeCode_DSTChangeApproaching(inStm);

	if (inStm->tm_isdst) {
		tcc.dcfTc.Z1 = 1;
		tcc.dcfTc.Z2 = 0;
	} else {
		tcc.dcfTc.Z1 = 0;
		tcc.dcfTc.Z2 = 1;
	}

	tcc.dcfTc.minute	= convertIntToTwoDigitBCD( inStm->tm_min );
	tcc.dcfTc.hour		= convertIntToTwoDigitBCD( inStm->tm_hour );
	tcc.dcfTc.dayOfMonth	= convertIntToTwoDigitBCD( inStm->tm_mday );
	tcc.dcfTc.dayOfWeek	= convertIntToTwoDigitBCD( wday );
	tcc.dcfTc.month		= convertIntToTwoDigitBCD( mon );
	tcc.dcfTc.year		= convertIntToTwoDigitBCD( inStm->tm_year );

	/* set parities */
	tcc.dcfTc.P1 = computeEvenParityBit( tcc.dcfTc.minute );
	tcc.dcfTc.P2 = computeEvenParityBit( tcc.dcfTc.hour   );
	unsigned c = 0u;
	c += computeEvenParityBit( tcc.dcfTc.dayOfMonth	);
	c += computeEvenParityBit( tcc.dcfTc.dayOfWeek	);
	c += computeEvenParityBit( tcc.dcfTc.month	);
	c += computeEvenParityBit( tcc.dcfTc.year	);
	tcc.dcfTc.P3 = (c % 2);

	timeCode_CopyOut(&tcc, pBlock);
}

static int
timeCode_DSTChangeApproaching(const struct tm * inStm)
{
	struct tm nextHourTM = *inStm;

	nextHourTM.tm_hour += 1;
	normalizeStructTM(&nextHourTM);

	return (inStm->tm_isdst != nextHourTM.tm_isdst);
}

static unsigned int
convertIntToTwoDigitBCD(unsigned int v)
{
	v %= 100u;

	unsigned hi = v / 10;
	unsigned lo = v % 10;

	return ((hi << 4) + lo);
}

static unsigned int
computeEvenParityBit(unsigned int v)
{
	unsigned i, mask = 1u, onesQty = 0;

	for (i = 0; i < (sizeof(v) * 8u); ++i) {
		if ( (v & mask) ) {
			++onesQty;
		}
		mask <<= 1;
	}

	return (onesQty % 2);
}

static const char  f0name[] = "M";
static const char  f0desc[] = "Start of minute";
static const char  f1name[] = "weather";
static const char  f1desc[] = "Weather info";
static const char  f2name[] = "R";
static const char  f2desc[] = "Abnormal transmitter operation";
static const char  f3name[] = "A1";
static const char  f3desc[] = "Summer time announcement";
static const char  f4name[] = "Z1";
static const char  f4desc[] = "CEST in effect";
static const char  f5name[] = "Z2";
static const char  f5desc[] = "CET in effect";
static const char  f6name[] = "A2";
static const char  f6desc[] = "Leap second announcement";
static const char  f7name[] = "S";
static const char  f7desc[] = "Start of encoded time";
static const char  f8name[] = "min";
static const char  f8desc[] = "Minutes 00-59";
static const char  f9name[] = "P1";
static const char  f9desc[] = "Even parity over minute bits";
static const char f10name[] = "hour";
static const char f10desc[] = "Hours 00-23";
static const char f11name[] = "P2";
static const char f11desc[] = "Even parity over hour bits";
static const char f12name[] = "dom";
static const char f12desc[] = "Day of month";
static const char f13name[] = "dow";
static const char f13desc[] = "Day of week (Mon=1, Sun=7)";
static const char f14name[] = "month";
static const char f14desc[] = "Month number 01-12";
static const char f15name[] = "year";
static const char f15desc[] = "Year within century 00-99";
static const char f16name[] = "P3";
static const char f16desc[] = "Parity over date bits";
static const char f17name[] = "-";
static const char f17desc[] = "Minute Mark (no AM)";

#define FIELDSPLIT_ROWS_QTY 18
#define FIELDSPLIT_BINSTR_SZ 16
#define FIELDSPLIT_HEXSTR_SZ 8

static struct {
	const unsigned	 offset;
	const unsigned	 length;
	const char	*name;
	const char	*desc;
	unsigned	 value;
	char		 valueAsBinStr[FIELDSPLIT_BINSTR_SZ];
	char		 valueAsHexStr[FIELDSPLIT_HEXSTR_SZ];
} fieldSplit[FIELDSPLIT_ROWS_QTY] = {
	{  0,   1,   f0name,   f0desc,  0,  "\0",  "\0" },
	{  1,  14,   f1name,   f1desc,  0,  "\0",  "\0" },
	{ 15,   1,   f2name,   f2desc,  0,  "\0",  "\0" },
	{ 16,   1,   f3name,   f3desc,  0,  "\0",  "\0" },
	{ 17,   1,   f4name,   f4desc,  0,  "\0",  "\0" },
	{ 18,   1,   f5name,   f5desc,  0,  "\0",  "\0" },
	{ 19,   1,   f6name,   f6desc,  0,  "\0",  "\0" },
	{ 20,   1,   f7name,   f7desc,  0,  "\0",  "\0" },
	{ 21,   7,   f8name,   f8desc,  0,  "\0",  "\0" },
	{ 28,   1,   f9name,   f9desc,  0,  "\0",  "\0" },
	{ 29,   6,  f10name,  f10desc,  0,  "\0",  "\0" },
	{ 35,   1,  f11name,  f11desc,  0,  "\0",  "\0" },
	{ 36,   6,  f12name,  f12desc,  0,  "\0",  "\0" },
	{ 42,   3,  f13name,  f13desc,  0,  "\0",  "\0" },
	{ 45,   5,  f14name,  f14desc,  0,  "\0",  "\0" },
	{ 50,   8,  f15name,  f15desc,  0,  "\0",  "\0" },
	{ 58,   1,  f16name,  f16desc,  0,  "\0",  "\0" },
	{ 59,   1,  f17name,  f17desc,  0,  "\0",  "\0" },
};

static const DCF77FieldViews_t  fieldsViews[FIELDSPLIT_ROWS_QTY] = {
    {	&fieldSplit[0].valueAsBinStr[0],
	&fieldSplit[0].valueAsHexStr[0],
	f0name,
	f0desc					},
    {	&fieldSplit[1].valueAsBinStr[0],
	&fieldSplit[1].valueAsHexStr[0],
	f1name,
	f1desc					},
    {	&fieldSplit[2].valueAsBinStr[0],
	&fieldSplit[2].valueAsHexStr[0],
	f2name,
	f2desc					},
    {	&fieldSplit[3].valueAsBinStr[0],
	&fieldSplit[3].valueAsHexStr[0],
	f3name,
	f3desc					},
    {	&fieldSplit[4].valueAsBinStr[0],
	&fieldSplit[4].valueAsHexStr[0],
	f4name,
	f4desc					},
    {	&fieldSplit[5].valueAsBinStr[0],
	&fieldSplit[5].valueAsHexStr[0],
	f5name,
	f5desc					},
    {	&fieldSplit[6].valueAsBinStr[0],
	&fieldSplit[6].valueAsHexStr[0],
	f6name,
	f6desc					},
    {	&fieldSplit[7].valueAsBinStr[0],
	&fieldSplit[7].valueAsHexStr[0],
	f7name,
	f7desc					},
    {	&fieldSplit[8].valueAsBinStr[0],
	&fieldSplit[8].valueAsHexStr[0],
	f8name,
	f8desc					},
    {	&fieldSplit[9].valueAsBinStr[0],
	&fieldSplit[9].valueAsHexStr[0],
	f9name,
	f9desc					},
    {	&fieldSplit[10].valueAsBinStr[0],
	&fieldSplit[10].valueAsHexStr[0],
	f10name,
	f10desc					},
    {	&fieldSplit[11].valueAsBinStr[0],
	&fieldSplit[11].valueAsHexStr[0],
	f11name,
	f11desc					},
    {	&fieldSplit[12].valueAsBinStr[0],
	&fieldSplit[12].valueAsHexStr[0],
	f12name,
	f12desc					},
    {	&fieldSplit[13].valueAsBinStr[0],
	&fieldSplit[13].valueAsHexStr[0],
	f13name,
	f13desc					},
    {	&fieldSplit[14].valueAsBinStr[0],
	&fieldSplit[14].valueAsHexStr[0],
	f14name,
	f14desc					},
    {	&fieldSplit[15].valueAsBinStr[0],
	&fieldSplit[15].valueAsHexStr[0],
	f15name,
	f15desc					},
    {	&fieldSplit[16].valueAsBinStr[0],
	&fieldSplit[16].valueAsHexStr[0],
	f16name,
	f16desc					},
    {	&fieldSplit[17].valueAsBinStr[0],
	&fieldSplit[17].valueAsHexStr[0],
	f17name,
	f17desc					},
};

void
DCF77TimeCode_SplitInFields(const DCF77Block_t * pBlock,
    const DCF77FieldViews_t * pFieldsViews[],
    size_t * fieldsViewsSz)
{
	breakBlockToTimecodeFields(pBlock);

	*pFieldsViews  = fieldsViews;
	*fieldsViewsSz = FIELDSPLIT_ROWS_QTY;
}

static void
breakBlockToTimecodeFields(const DCF77Block_t * pBlock)
{
	char blockBits[DCF77TIMECODE_BITS_QTY];

	splitBlockToStringOfBits(pBlock, blockBits);
	regroupBitsIntoTimecodeFields(blockBits);
}

static void
splitBlockToStringOfBits(const DCF77Block_t * pBlock, char outBits[])
{
	unsigned i;

	for (i = 0; i < DCF77TIMECODE_BITS_QTY; ++i) {
		unsigned byteIdx = i / 8;
		unsigned bitMask = 1u << (i % 8);

		outBits[i] = (pBlock->data[byteIdx] & bitMask) ? 1 : 0;
	}
}

static void
regroupBitsIntoTimecodeFields(const char blockBits[])
{
	unsigned i, j;

	for (i = 0; i < FIELDSPLIT_ROWS_QTY; ++i) {
		unsigned offset = fieldSplit[i].offset;
		unsigned length = fieldSplit[i].length;
		unsigned collected = 0u;
		unsigned mask = 1u;

		for (j = 0; j < length; ++j) {
			unsigned currBit = blockBits[offset + j];

			if (currBit) {
				fieldSplit[i].valueAsBinStr[j] = '1';
				collected |= mask;
			} else {
				fieldSplit[i].valueAsBinStr[j] = '0';
			}

			mask <<= 1;
		}
		fieldSplit[i].value = collected;
		fieldSplit[i].valueAsBinStr[j] = '\0';

		snprintf(fieldSplit[i].valueAsHexStr, FIELDSPLIT_HEXSTR_SZ,
			"%x", fieldSplit[i].value);
	}
}
