#ifndef D_DCF77TimeCodePrivate_h
#define D_DCF77TimeCodePrivate_h

#include "DCF77Block.h"

enum {
	DCF77TIMECODE_BITS_QTY = 60
};

typedef struct __attribute__((packed)) DCF77TimeCode_t {
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
} DCF77TimeCode_t;

union TimeCodeConversion_t {
	DCF77TimeCode_t dcfTc;
	DCF77Block_t	block;
};

#endif /* #ifndef D_DCF77TimeCodePrivate_h */
