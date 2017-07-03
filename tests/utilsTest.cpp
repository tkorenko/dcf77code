#include "CppUTest/TestHarness.h"
#include <time.h>
extern "C"
{
#include "utils.h"
#include "string.h"
};

TEST_GROUP(AStructTMNormalizer)
{
	struct tm stm;
	enum {
		DST_UNSPECIFIED = -1,
		DST_DEASSERTED = 0,
		DST_ASSERTED = 1
	};

	void setup() override {
		memset((void*)&stm, 0, sizeof(stm));
		stm.tm_isdst = -1;
		stm.tm_zone = NULL;
	}

	void SET_HUMAN_TIMESTAMP(int year, int mon, int day,
	    int hour, int min, int sec) {
		SET_RAW_TIMESTAMP((year - 1900), (mon - 1), day,
		    hour, min, sec);
	}

	void SET_RAW_TIMESTAMP(int year, int mon, int day,
	    int hour, int min, int sec) {
		stm.tm_year = year;
		stm.tm_mon  = mon;
		stm.tm_mday = day;
		stm.tm_hour = hour;
		stm.tm_min  = min;
		stm.tm_sec  = sec;
		stm.tm_isdst = -1;
		stm.tm_zone = NULL;
	}

	void CHECK_HUMAN_TIMESTAMP(int year, int mon, int day,
	    int hour, int min, int sec) {
		CHECK_RAW_TIMESTAMP((year - 1900), (mon - 1), day,
		    hour, min, sec);
	}

	void CHECK_RAW_TIMESTAMP(int year, int mon, int day,
	    int hour, int min, int sec) {
		LONGS_EQUAL(year, stm.tm_year);
		LONGS_EQUAL(mon,  stm.tm_mon);
		LONGS_EQUAL(day,  stm.tm_mday);
		LONGS_EQUAL(hour, stm.tm_hour);
		LONGS_EQUAL(min,  stm.tm_min);
		LONGS_EQUAL(sec,  stm.tm_sec);
	}

	void CHECK_DST(int dst) {
		LONGS_EQUAL(dst,  stm.tm_isdst);
	}
};

TEST(AStructTMNormalizer, StmIsClearedOnInit) {
	CHECK_RAW_TIMESTAMP(0, 0, 0, 0, 0, 0);
	CHECK_DST( DST_UNSPECIFIED );
}

TEST(AStructTMNormalizer, CheckHelperMethods1) {
	SET_HUMAN_TIMESTAMP(2017, 2, 3, 4, 5, 6); /* yy mm dd HH MM SS */

	CHECK_RAW_TIMESTAMP( 117, 1, 3, 4, 5, 6);
	CHECK_DST( DST_UNSPECIFIED );
}

TEST(AStructTMNormalizer, CheckHelperMethods2) {
	SET_RAW_TIMESTAMP    ( 117, 1, 3, 4, 5, 6); /* yy mm dd HH MM SS */

	CHECK_HUMAN_TIMESTAMP(2017, 2, 3, 4, 5, 6);
	CHECK_DST( DST_UNSPECIFIED );
}

TEST(AStructTMNormalizer, CorrectsMinutesOverflow) {
	SET_HUMAN_TIMESTAMP  (2017, 7, 8,  9, 60, 0);

	normalizeStructTM(&stm);

	CHECK_HUMAN_TIMESTAMP(2017, 7, 8, 10,  0, 0);
}

TEST(AStructTMNormalizer, CorrectsMinutesUnderflow) {
	SET_HUMAN_TIMESTAMP  (2017, 7, 8, 9, -1, 0);

	normalizeStructTM(&stm);

	CHECK_HUMAN_TIMESTAMP(2017, 7, 8, 8, 59, 0);
}

TEST(AStructTMNormalizer, CorrectsHoursOverflow) {
	SET_HUMAN_TIMESTAMP  (2017, 7, 8, 24, 10, 0);

	normalizeStructTM(&stm);

	CHECK_HUMAN_TIMESTAMP(2017, 7, 9,  0, 10, 0);
}

TEST(AStructTMNormalizer, CorrectsHoursUnderflow) {
	SET_HUMAN_TIMESTAMP  (2017, 7, 8, -1, 10, 0);

	normalizeStructTM(&stm);

	CHECK_HUMAN_TIMESTAMP(2017, 7, 7, 23, 10, 0);
}

/* XXX
 * The following tests depend on availability of DST for your local timezone.
 */
TEST(AStructTMNormalizer, AssertsDSTForSummerTime) {
	SET_HUMAN_TIMESTAMP(2017, 7, 8, 9, 10, 0);

	normalizeStructTM(&stm);

	CHECK_DST( DST_ASSERTED );
}

TEST(AStructTMNormalizer, DeassertsDSTForWinterTime) {
	SET_HUMAN_TIMESTAMP(2017, 12, 11, 10, 10, 0);

	normalizeStructTM(&stm);

	CHECK_DST( DST_DEASSERTED );
}
