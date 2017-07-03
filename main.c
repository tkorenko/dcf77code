#include <stdio.h>
#include <stdint.h>

struct __attribute__((packed)) DCF77TimeCode {
	unsigned	M:1;		/* Start of minute. Always 0 */
	unsigned	weather:14;	/* junk */
	unsigned	R:1;		/* 1 - Abnormal tx operation */
	unsigned	A1:1;		/* 1 - Summer Time announcement */
	unsigned	Z1:1;		/* 1 - CEST in effect */
	unsigned	Z2:1;		/* 1 - CET in effect */
	unsigned	A2:1;		/* Leap second announcement */
	unsigned	S:1;		/* Start of time: Always 1 */
	unsigned	minute:7;	/* BCD: minutes 00-59 */
	unsigned	P1:1;		/* Even parity over minute bits */
	unsigned	hour:6;		/* BCD: hours 0-23 */
	unsigned	P2:1;		/* Even parity over hour bits */
	unsigned	dayOfMonth:6;	/* BCD: Day of month */
	unsigned	dayOfWeek:3;	/* Monday=1, Sunday=7 */
	unsigned	month:5;	/* BCD: Month number 01-12 */
	unsigned	year:8;		/* BCD: Year 00-99 */
	unsigned	P3:1;		/* Even parity from dayOfMonth */
	unsigned	MM:1;		/* minute mark - no modulation */
};

union TimeCodeConversion {
	struct DCF77TimeCode tc;
	uint8_t raw[8];
};

static inline union TimeCodeConversion *
convertTimeCodePtr(struct DCF77TimeCode * p)
{
	return (union TimeCodeConversion *)p;
}

void
DCF77TimeCode_InitStruct(struct DCF77TimeCode *pTc)
{
	unsigned i;
	union TimeCodeConversion * pTcc =
		convertTimeCodePtr(pTc);

	if (NULL == pTc)
		return;

	for (i = 0; i < 8; ++i) {
		pTcc->raw[i] = 0x00;
	}

	pTcc->tc.S  = 1;
	pTcc->tc.MM = 0;
}

void
DCF77TimeCode_DumpAsBinary(struct DCF77TimeCode *pTc)
{
	union TimeCodeConversion * tcc =
		convertTimeCodePtr(pTc);

	printf("---- DumpAsBinary(%p) ----\n", (void*)pTc);

	unsigned y, x, mask;
	for (y = 0; y < 64; ++y) {
		printf("%d", (y % 10));
		if (7 == (y % 8))
			printf(" ");
	}
	printf("\n");
	for (y = 0; y < 8; ++y) {
		mask = 1u;
		for (x = 0; x < 8; ++x) {
			unsigned bit = tcc->raw[y] & mask;
			mask <<= 1;
			printf("%c", ((bit) ? '1' : '0') );
		}
		printf(" ");
	}
	printf("\n");
}

static void
DCF77TimeCode_DumpAsCArrUint8_t(struct DCF77TimeCode *pTc)
{
	union TimeCodeConversion * tcc =
		convertTimeCodePtr(pTc);

	printf("---- DumpAsCArrUint8_t (%p) ----\n", (void*)pTc);

	unsigned y;
	printf("# {");
	for (y = 0; y < 8; ++y) {
		unsigned byte = tcc->raw[y];
		printf(" 0x%02Xu", byte);
		if (y < 7)
			printf(",");
	}
	printf("}\n");
}

void
DCF77TimeCode_DumpAsAsciiCmd(struct DCF77TimeCode *pTc)
{
	union TimeCodeConversion * tcc =
		convertTimeCodePtr(pTc);

	printf("---- DumpAsAsciiCmd (%p) ----\n", (void*)pTc);

	unsigned y;
	printf("# bw");
	for (y = 0; y < 8; ++y) {
		unsigned byte = tcc->raw[y];
		printf("%02X", byte);
	}
	printf("\n");
}

static inline unsigned int
mkEvenParity(unsigned int v)
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

static inline unsigned int
mkTwoDigitBCD(unsigned v)
{
	v %= 100u;

	unsigned hi = v / 10;
	unsigned lo = v % 10;

	return ((hi << 4) + lo);
}

void
DCF77TimeCode_SetMinute(struct DCF77TimeCode *pTc, unsigned minute)
{
	if (NULL == pTc)
		return;

	unsigned bcdMinute = mkTwoDigitBCD( minute );
	printf("---- DCF77TimeCode_SetMinute(%02d) ----\n", minute);

	pTc->minute = bcdMinute;
}

void
DCF77TimeCode_SetHour(struct DCF77TimeCode *pTc, unsigned hour)
{
	if (NULL == pTc)
		return;

	unsigned bcdHour = mkTwoDigitBCD( hour );
	printf("---- DCF77TimeCode_SetHour(%02d) ----\n", hour);

	pTc->hour = bcdHour;
}

void
DCF77TimeCode_SetDayOfMonth(struct DCF77TimeCode *pTc, unsigned dom)
{
	if (NULL == pTc)
		return;

	unsigned bcdDom = mkTwoDigitBCD( dom );
	printf("---- DCF77TimeCode_SetDayOfMonth(%02d) ----\n", dom);

	pTc->dayOfMonth = bcdDom;
}

void
DCF77TimeCode_SetDayOfWeek(struct DCF77TimeCode *pTc, unsigned dow)
{
	if (NULL == pTc)
		return;

	unsigned bcdDow = mkTwoDigitBCD( dow );
	printf("---- DCF77TimeCode_SetDayOfWeek(%02d) ----\n", dow);

	pTc->dayOfWeek = bcdDow;
}

void
DCF77TimeCode_SetMonth(struct DCF77TimeCode *pTc, unsigned month)
{
	if (NULL == pTc)
		return;

	unsigned bcdMonth = mkTwoDigitBCD( month );
	printf("---- DCF77TimeCode_SetMonth(%02d) ----\n", month);

	pTc->month = bcdMonth;
}

void
DCF77TimeCode_SetYear(struct DCF77TimeCode *pTc, unsigned year)
{
	if (NULL == pTc)
		return;

	unsigned bcdYear = mkTwoDigitBCD( year );
	printf("---- DCF77TimeCode_SetYear(%02d) ----\n", year);

	pTc->year = bcdYear;
}


void DCF77TimeCode_SetParities(struct DCF77TimeCode *pTc)
{
	if (NULL == pTc)
		return;

	pTc->P1 = mkEvenParity(pTc->minute);
	pTc->P2 = mkEvenParity(pTc->hour);

	unsigned c = 0u;
	c += mkEvenParity(pTc->dayOfMonth);
	c += mkEvenParity(pTc->dayOfWeek);
	c += mkEvenParity(pTc->month);
	c += mkEvenParity(pTc->year);
	pTc->P3 = (c % 2);

	printf("---- DCF77TimeCode_SetParities(): %d %d %d ----\n",
		pTc->P1, pTc->P2, pTc->P3);
}

void
DCF77TimeCode_Compose(struct DCF77TimeCode *pTc,
    unsigned minute, unsigned hour, unsigned dayOfMonth,
    unsigned dayOfWeek, unsigned month, unsigned year)
{
	DCF77TimeCode_InitStruct(pTc);

	//DCF77TimeCode_DumpAsBinary(pTc);

	DCF77TimeCode_SetMinute(pTc, minute);

	DCF77TimeCode_SetHour(pTc, hour);

	DCF77TimeCode_SetDayOfMonth(pTc, dayOfMonth);

	DCF77TimeCode_SetDayOfWeek(pTc, dayOfWeek);

	DCF77TimeCode_SetMonth(pTc, month);

	DCF77TimeCode_SetYear(pTc, year);

	DCF77TimeCode_SetParities(pTc);

	//DCF77TimeCode_DumpAsBinary(pTc);
}

int
main(int argc, char * argv[])
{
	struct DCF77TimeCode tc;
#if 0
	union TimeCodeConversion * pTcc = convertTimeCodePtr(&tc);

	printf("sizeof(DCF77TimeCode): %u\n", sizeof(tc));

	{
		unsigned i;
		for (i = 0; i < 8; ++i) {
			pTcc->raw[i] = 0xFFu;
		}
	}

	DCF77TimeCode_DumpAsBinary(&tc);
	DCF77TimeCode_InitStruct(&tc);
	DCF77TimeCode_DumpAsBinary(&tc);

	unsigned minute = 1;
	printf("\n");
	DCF77TimeCode_SetMinute(&tc, minute);
	//DCF77TimeCode_DumpAsBinary(&tc);

	minute = 42;
	printf("\n");
	DCF77TimeCode_SetMinute(&tc, minute);
	//DCF77TimeCode_DumpAsBinary(&tc);

	unsigned hour = 13;
	printf("\n");
	DCF77TimeCode_SetHour(&tc, hour);
	//DCF77TimeCode_DumpAsBinary(&tc);

	unsigned dayOfMonth = 28;
	printf("\n");
	DCF77TimeCode_SetDayOfMonth(&tc, dayOfMonth);
	//DCF77TimeCode_DumpAsBinary(&tc);

	unsigned dayOfWeek = 3;
	printf("\n");
	DCF77TimeCode_SetDayOfWeek(&tc, dayOfWeek);
	//DCF77TimeCode_DumpAsBinary(&tc);

	unsigned month = 6;
	printf("\n");
	DCF77TimeCode_SetMonth(&tc, month);
	//DCF77TimeCode_DumpAsBinary(&tc);

	unsigned year = 16;
	printf("\n");
	DCF77TimeCode_SetYear(&tc, year);
	//DCF77TimeCode_DumpAsBinary(&tc);

	printf("\n");
	DCF77TimeCode_SetParities(&tc);
	DCF77TimeCode_DumpAsBinary(&tc);

	DCF77TimeCode_DumpAsUint8_t(&tc);
#endif

	unsigned i;
	for (i = 0; i < 20; ++i) {
		DCF77TimeCode_Compose(&tc, (25+i), 14, 3, 1, 7, 17);
		DCF77TimeCode_DumpAsAsciiCmd(&tc);
	}

	return 0;
}



