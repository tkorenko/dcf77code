#include "CppUTest/TestHarness.h"
#include <time.h>
#include <string.h>
extern "C"
{
#include "DCF77TimeCode.h"
#include "DCF77TimeCodePrivate.h"
};

struct TimeCodeConversionTestsBase : public Utest
{
	union TimeCodeConversion_t tcc;

	void SetTCCToAllOnes() {
		for (int i = 0; i < DCF77BLOCK_SIZE; ++i) {
			tcc.block.data[i] = 0xFFu;
		}
	}

	void SetTCCToAllZeros() {
		for (int i = 0; i < DCF77BLOCK_SIZE; ++i) {
			tcc.block.data[i] = 0x00u;
		}
	}

	void CHECK_TIMECODE_M_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.M);
	}

	void CHECK_TIMECODE_WEATHER_ZEROED() {
		CHECK(0u == tcc.dcfTc.weather);
	}

	void CHECK_TIMECODE_R_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.R);
	}

	void CHECK_TIMECODE_A1_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.A1);
	}

	void CHECK_TIMECODE_A2_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.A2);
	}

	void CHECK_TIMECODE_Z1_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.Z1);
	}

	void CHECK_TIMECODE_Z2_DEASSERTED() {
		CHECK(0u == tcc.dcfTc.Z2);
	}

	void CHECK_TIMECODE_S_ASSERTED() {
		CHECK(1u == tcc.dcfTc.S);
	}
};


/* ====================================================================== */
TEST_GROUP_BASE(ATimeCodeConversionHelper, TimeCodeConversionTestsBase)
{
};

TEST(ATimeCodeConversionHelper, SetsBitsOfBlockToAllOnes) {
	SetTCCToAllOnes();

	for (int i = 0; i < DCF77BLOCK_SIZE; ++i) {
		BYTES_EQUAL(0xFFu, tcc.block.data[i]);
	}
}

TEST(ATimeCodeConversionHelper, SetsBitsOfBlockToAllZeros) {
	SetTCCToAllZeros();

	for (int i = 0; i < DCF77BLOCK_SIZE; ++i) {
		BYTES_EQUAL(0x00u, tcc.block.data[i]);
	}
}


/* ====================================================================== */
TEST_GROUP_BASE(ATimeCode_Init, TimeCodeConversionTestsBase)
{
	void setup() override {
		SetTCCToAllOnes();
	}

	void CHECK_TIMECODE_INIT_VALUES() {
		CHECK_TIMECODE_M_DEASSERTED();
		CHECK_TIMECODE_WEATHER_ZEROED();
		CHECK_TIMECODE_R_DEASSERTED();
		CHECK_TIMECODE_A1_DEASSERTED();
		CHECK_TIMECODE_Z1_DEASSERTED();
		CHECK_TIMECODE_Z2_DEASSERTED();
		CHECK_TIMECODE_A2_DEASSERTED();
		CHECK_TIMECODE_S_ASSERTED();
	}
};

TEST(ATimeCode_Init, SetsTimeCodeBitsToPredefinedValues) {
	DCF77TimeCode_Init(&tcc.block);

	CHECK_TIMECODE_INIT_VALUES();
}


/* ====================================================================== */
TEST_GROUP_BASE(AStructTMToTimeCodeConversion, TimeCodeConversionTestsBase)
{
	struct tm stm;

	void setup() override {
		DCF77TimeCode_Init(&tcc.block);
		memset((void*)&stm, 0, sizeof(stm));
	}

	int convertTwoBytesBCDToInt(int v) {
		int hi = (v & 0xF0) >> 4;
		int lo = (v & 0x0F);

		return 10 * hi + lo;
	}

	void CHECK_TIMECODE_MINUTES_EQUAL(int expected) {
		int m = convertTwoBytesBCDToInt(tcc.dcfTc.minute);
		BYTES_EQUAL(expected, m);
	}

	void CHECK_TIMECODE_HOURS_EQUAL(int expected) {
		int h = convertTwoBytesBCDToInt(tcc.dcfTc.hour);
		BYTES_EQUAL(expected, h);
	}

	void CHECK_TIMECODE_DAYOFMONTH_EQUAL(int expected) {
		int dom = convertTwoBytesBCDToInt(tcc.dcfTc.dayOfMonth);
		BYTES_EQUAL(expected, dom);
	}

	void CHECK_TIMECODE_MONTH_EQUAL(int expected) {
		int m = convertTwoBytesBCDToInt(tcc.dcfTc.month);
		BYTES_EQUAL(expected, m);
	}

	void CHECK_TIMECODE_YEAR_EQUAL(int expected) {
		int y = convertTwoBytesBCDToInt(tcc.dcfTc.year);
		BYTES_EQUAL(expected, y);
	}
};

TEST(AStructTMToTimeCodeConversion, PassesMinutes) {
	for (int m = 0; m < 60; ++m) {
		stm.tm_min = m;

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		CHECK_TIMECODE_MINUTES_EQUAL(m);
	}
}

TEST(AStructTMToTimeCodeConversion, PassesHours) {
	for (int h = 0; h < 24; ++h) {
		stm.tm_hour = h;

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		CHECK_TIMECODE_HOURS_EQUAL(h);
	}
}

TEST(AStructTMToTimeCodeConversion, PassesDayOfMonth) {
	for (int dom = 1; dom < 32; ++dom) {
		stm.tm_mday = dom;

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		CHECK_TIMECODE_DAYOFMONTH_EQUAL(dom);
	}
}

TEST(AStructTMToTimeCodeConversion, DayOfWeekHandling_Sunday) {
	stm.tm_wday = 0;	/* 0 is Sunday (c) ctime(3) */

	DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

	BYTES_EQUAL(7, tcc.dcfTc.dayOfWeek); /* 7 is Sunday (c) wiki/DCF77 */
}

TEST(AStructTMToTimeCodeConversion, DayOfWeekHandling_OtherDays) {
	for (int dow = 1; dow < 7; ++dow) {
		stm.tm_wday = dow;

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		BYTES_EQUAL(dow, tcc.dcfTc.dayOfWeek);
	}
}

TEST(AStructTMToTimeCodeConversion, PassesMonthNumbersAdvancedByOne) {
	for (int m = 0; m < 12; ++m) {
		stm.tm_mon = m;	/* strcut tm: 0 is January */

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		CHECK_TIMECODE_MONTH_EQUAL(m + 1);
	}
}

TEST(AStructTMToTimeCodeConversion, PassesYears) {
	for (int y = 0; y < 100; ++y) {
		stm.tm_year = y;

		DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

		CHECK_TIMECODE_YEAR_EQUAL(y);
	}
}

TEST(AStructTMToTimeCodeConversion, PropagatesAssertedDST) {
	stm.tm_isdst = 1;

	DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

	LONGS_EQUAL(1, tcc.dcfTc.Z1);
	LONGS_EQUAL(0, tcc.dcfTc.Z2);
}

TEST(AStructTMToTimeCodeConversion, PropagatesDeassertedDST) {
	stm.tm_isdst = 0;

	DCF77TimeCode_ConvertFromStructTM(&tcc.block, &stm);

	LONGS_EQUAL(0, tcc.dcfTc.Z1);
	LONGS_EQUAL(1, tcc.dcfTc.Z2);
}
