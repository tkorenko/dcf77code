#ifndef D_DCF77TimeCode_h
#define D_DCF77TimeCode_h

#include <time.h>
#include "DCF77Block.h"

typedef struct {
	const char *asBinStr;
	const char *asHexStr;
	const char *name;
	const char *nameDescr;
} DCF77FieldViews_t;

void DCF77TimeCode_Init(DCF77Block_t * pBlock);
void DCF77TimeCode_ConvertToStructTM(const DCF77Block_t * pBlock,
	struct tm * outStm);
void DCF77TimeCode_ConvertFromStructTM(DCF77Block_t * pBlock,
	const struct tm * inStm);
void DCF77TimeCode_SplitInFields(const DCF77Block_t * pBlock,
	const DCF77FieldViews_t * pFieldsViews[],
	size_t * fieldsViewsSz);

#endif /* #ifndef D_DCF77TimeCode_h */
