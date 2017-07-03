#include <ctype.h>
#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "DCF77Block.h"
#include "DCF77TimeCode.h"
#include "utils.h"

#define PROGNAME "dcfcode"

static enum {
	OP_MODE_UNSPECIFIED,
	OP_MODE_CREATE_BLOCK,
	OP_MODE_DUMP_BLOCK,
	OP_MODE_DETAILED_DUMP
} opMode = OP_MODE_UNSPECIFIED;

static int startOffset  = 0;
static int createBlocks = 1;
static const char * timeSpec = NULL;
static const char * dumpTimeFormat = "%c (%Z)";

static void printUsage(void);
static void processCreateBlockCmd(int argc, char * argv[]);
static void processDumpBlockCmd(int argc, char * argv[]);
static void processDetailedDumpCmd(int argc, char * argv[]);
static void parseTimeSpec(const char * text, struct tm * pStm);

int
main(int argc, char * argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "cDdf:n:s:t:")) != -1) {
		switch (ch) {
		case 'c':
			opMode = OP_MODE_CREATE_BLOCK;
			break;
		case 'd':
			opMode = OP_MODE_DUMP_BLOCK;
			break;
		case 'D':
			opMode = OP_MODE_DETAILED_DUMP;
			break;
		case 'f':
			dumpTimeFormat = optarg;
			break;
		case 'n':
			createBlocks = (int)strtol(optarg, NULL, 10);
			break;
		case 's':
			startOffset = (int)strtol(optarg, NULL, 10);
			break;
		case 't':
			timeSpec = optarg;
			break;
		}
	}
	argc -= optind;
	argv += optind;

	tzset();

	switch (opMode) {
	case OP_MODE_UNSPECIFIED:
		printUsage();
		/* NOTREACHED */
		break;
	case OP_MODE_CREATE_BLOCK:
		processCreateBlockCmd(argc, argv);
		break;
	case OP_MODE_DUMP_BLOCK:
		processDumpBlockCmd(argc, argv);
		break;
	case OP_MODE_DETAILED_DUMP:
		processDetailedDumpCmd(argc, argv);
		break;
	}

	return 0;
}

#if 0
static void
cfgDump(void)
{
	printf("# -- cfgDump() ---\n");
	printf("  opMode        : %d\n", opMode);
	printf("  startOffset   : %d\n", startOffset);
	printf("  createBlocks  : %d\n", createBlocks);
	printf("  timeSpec      : %p\n", (void*)timeSpec);
	if (NULL != timeSpec) {
		printf("                  '%s'\n", timeSpec);
	}
	printf("  dumpTimeFormat: %p\n", (void*)dumpTimeFormat);
	if (NULL != dumpTimeFormat) {
		printf("                  '%s'\n", dumpTimeFormat);
	}
	printf("# ================\n");
}
#endif /* #if 0 */

static void
printUsage(void)
{
	fprintf(stderr, "*** %s: Incorrect usage ***\n", PROGNAME);
	fprintf(stderr,
	    "\n"
	    "    Mode of operation is selected by:\n"
	    "  %% dcfcode { -c | -d | -D } ...\n"
	    "\n"
	    "    To create a block, use:\n"
	    "  %% dcfcode -c [-t <timespec>] [-s <offset>] [-n <repeat>]\n"
	    "    To dump a block, run:\n"
	    "  %% dcfcode -d [-f <time_format>] <block1> [<blockN>]\n"
	    "    To split a block in bits, use:\n"
	    "  %% dcfcode -D <block1> [<blockN>]\n"
	    "    where:\n"
	    "    -t { [[[[yy]mm]dd]HH]MM | <block> }\n"
	    "    -s { [+]<minutes> | -<minutes> }\n"
	    "    -f <according to strftime(3)>\n"
	);

	exit(EX_USAGE);
}

static void createBlockAt(const struct tm * pStm);
static void
advanceTimeByMinutes(struct tm * pStm, int minutes)
{
	pStm->tm_min += minutes;

	normalizeStructTM(pStm);
}

static void
processCreateBlockCmd(int argc, char * argv[])
{
	struct tm stm;
	int i;

	parseTimeSpec(timeSpec, &stm);
	advanceTimeByMinutes(&stm, startOffset);

	for (i = 0; i < createBlocks; ++i) {
		createBlockAt(&stm);
		advanceTimeByMinutes(&stm, 1);
	}
}

#define BLOCK_TEXT_SZ (DCF77BLOCK_TEXT_LEN + 1)
static void
createBlockAt(const struct tm * pStm)
{
	char textBlock[BLOCK_TEXT_SZ];
	DCF77Block_t block;

	/* 1. convert to DCF77Block_t  */
	DCF77TimeCode_ConvertFromStructTM(&block, pStm);

	/* 2. convert Bin to Text */
	DCF77Block_ToText(&block, textBlock, BLOCK_TEXT_SZ);

	printf("%s\n", textBlock);
}

static void getCurrentTime(struct tm * pStm);
static void parseAsYYMMDDHHMM(const char * userInput, struct tm * pStm);
static void parseAsDCF77Block(const char * userInput, struct tm * pStm);

static void
parseTimeSpec(const char * text, struct tm * pStm)
{
	size_t textLen = 0u;

	if (NULL == text) {
		getCurrentTime(pStm);

		return;
	}

	textLen = strlen(text);
	switch (textLen) {
	case 2:	; case 4: ; case 6: ; case 8: ; case 10:
		parseAsYYMMDDHHMM(text, pStm);
		break;
	case 16:
		parseAsDCF77Block(text, pStm);
		break;
	default:
		errx(EX_DATAERR, "invalid timespec length");
		/* NOTREACHED */
		break;
	}
}

static void
getCurrentTime(struct tm * pStm)
{
	time_t t = time(NULL);

	if ((time_t)-1 == t) {
		err(EX_SOFTWARE, "time(3) failed");
		/* NOTREACHED */
	}

	(void)localtime_r(&t, pStm);
	pStm->tm_sec = 0;
}

static void fillOutCurrentTimestamp(char * buf, size_t bufSz);
static void copyStringBackToForth(char * dst, const char * src);
static void convertTextTimestampToStructTM(const char * textTS,
	struct tm * pStm);

#define TEXTTIMESTAMP_SZ 15

static void
parseAsYYMMDDHHMM(const char * userInput, struct tm * pStm)
{
	char textTimestamp[TEXTTIMESTAMP_SZ + 1];

	fillOutCurrentTimestamp(textTimestamp, TEXTTIMESTAMP_SZ);

	copyStringBackToForth(textTimestamp, userInput);

	convertTextTimestampToStructTM(textTimestamp, pStm);
}

static void
fillOutCurrentTimestamp(char * buf, size_t bufSz)
{
	time_t currTime = time(NULL);
	struct tm stm;

	(void)localtime_r(&currTime, &stm);

	(void)strftime(buf, bufSz, "%y%m%d%H%M", &stm);
}

static void
copyStringBackToForth(char * dst, const char * src)
{
	const char *s = src;
	char *d = dst;

	s += strlen(src) - 1;
	d += strlen(dst) - 1;

	while ( (s >= src) && (d >= dst) ) {
		*d = *s;
		--s;
		--d;
	}
}

#ifndef AVOID_STRPTIME_CONVERSION

static void
convertTextTimestampToStructTM(const char * textTS, struct tm * pStm)
{
	struct tm stm;

	memset((void*)&stm, 0, sizeof(stm));

	(void)strptime(textTS, "%y%m%d%H%M", &stm);

	stm.tm_isdst = -1;

	*pStm = stm;
}

#else  /* #ifndef AVOID_STRPTIME_CONVERSION */

static long strtol2bytes(const char * text);

static void
convertTextTimestampToStructTM(const char * textTS, struct tm * pStm)
{
	struct tm stm;

	stm.tm_year = strtol2bytes( &textTS[0] ) + 100;
	stm.tm_mon  = strtol2bytes( &textTS[2] ) - 1;
	stm.tm_mday = strtol2bytes( &textTS[4] );
	stm.tm_hour = strtol2bytes( &textTS[6] );
	stm.tm_min  = strtol2bytes( &textTS[8] );
	/* clear the rest of struct tm */
	stm.tm_sec  = 0;
	stm.tm_isdst = -1;	/* recalculated by mktime */
	stm.tm_zone = NULL;
	stm.tm_gmtoff = 0;

	*pStm = stm;
}

static long
strtol2bytes(const char * text)
{
	char locBuf[3];

	locBuf[0] = text[0];
	locBuf[1] = text[1];
	locBuf[2] = '\0';

	return strtol(locBuf, NULL, 10);
}

#endif /* #ifndef AVOID_STRPTIME_CONVERSION */

static void
parseAsDCF77Block(const char * userInput, struct tm * pStm)
{
	struct tm stm;
	DCF77Block_t block;

	DCF77Block_FromText(userInput, &block);

	DCF77TimeCode_ConvertToStructTM(&block, &stm);

	*pStm = stm;
}

#define CTBUF_SZ 80
static void
processDumpBlockCmd(int argc, char * argv[])
{
	DCF77Block_t block;
	struct tm stm;
	char ctBuf[CTBUF_SZ];
	int i;

	for (i = 0; i < argc; ++i) {
		ctBuf[0] = '\0';

		DCF77Block_FromText(argv[i], &block);
		DCF77TimeCode_ConvertToStructTM(&block, &stm);

		if (NULL == dumpTimeFormat) {
			asctime_r(&stm, ctBuf);
		} else {
			if (0u == strftime(ctBuf, CTBUF_SZ,
					dumpTimeFormat, &stm)) {
				/* strftime() produced nothing, thus: */
				ctBuf[0] = '\0';
			}
			strncat(ctBuf, "\n", CTBUF_SZ);
		}

		printf("%s -> %s", argv[i], ctBuf);
	}
}

static void
dumpBlockDetailed(const char * pBlock);

static void
processDetailedDumpCmd(int argc, char * argv[])
{
	int i;

	for (i = 0; i < argc; ++i) {
		printf("# %s\n", argv[i]);
		dumpBlockDetailed(argv[i]);
	}
}

static void printFieldViews(const DCF77FieldViews_t * pFieldView);

static void
dumpBlockDetailed(const char * pBlock)
{
	DCF77Block_t block;
	const DCF77FieldViews_t * pFieldsViews = NULL;
	size_t fieldsQty = 0u;

	DCF77Block_FromText(pBlock, &block);

	DCF77TimeCode_SplitInFields(&block, &pFieldsViews, &fieldsQty);

	if (NULL == pFieldsViews) {
		return;
	}

	for (size_t i = 0; i < fieldsQty; ++i) {
		printFieldViews(&pFieldsViews[i]);
		printf("\n");
	}
}

static const char * safeStr(const char * str);

static void
printFieldViews(const DCF77FieldViews_t * pFieldView)
{
	if (NULL == pFieldView) {
		printf("<NULL>");
		return;
	}

	printf("%14s : %4s : %-7s : %s",
	    safeStr(pFieldView->asBinStr), safeStr(pFieldView->asHexStr),
	    safeStr(pFieldView->name),     safeStr(pFieldView->nameDescr));
}

static const char *
safeStr(const char * str)
{
	return ((NULL == str) ? "#" : str);
}
