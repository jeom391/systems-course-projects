/***************************************************************
*      scanner routine for Mini C language                    *
*                                   2003. 3. 10               *
***************************************************************/

#pragma once


#define NO_KEYWORD 16	//16개로 확장
#define ID_LENGTH 12
#define STRING_LENGTH 256	// 문자열 리터럴 최대 길이

struct tokenType {
	int number;
	union {
		char id[ID_LENGTH];
		int num;
		char ch;	// 문자 리터럴 값
		double dnum;	// 실수 리터럴 값
		char str[STRING_LENGTH];	// 문자열 리터럴 값
	} value;
};


enum tsymbol {
	tnull = -1,
	tnot, tnotequ, tremainder, tremAssign, tident, tnumber,
	/* 0          1            2         3            4          5     */
	tand, tlparen, trparen, tmul, tmulAssign, tplus,
	/* 6          7            8         9           10         11     */
	tinc, taddAssign, tcomma, tminus, tdec, tsubAssign,
	/* 12         13          14        15           16         17     */
	tdiv, tdivAssign, tsemicolon, tless, tlesse, tassign,
	/* 18         19          20        21           22         23     */
	tequal, tgreat, tgreate, tlbracket, trbracket, teof,
	/* 24         25          26        27           28         29     */
	//   ...........    word symbols ................................. //
	/* 30         31          32        33           34         35     */
	tconst, telse, tif, tint, treturn, tvoid,
	/* 36         37          38        39                             */
	twhile,
	// 추가 키워드 토큰
	tchar, tdouble, tfor, tdo, tgoto, tswitch, tcase, tbreak, tdefault,
	// 추가 리터럴 토큰
	tcharLiteral, tstringLiteral, tdoubleLiteral,
	tlbrace, tor, trbrace
};


struct tokenType scanner();
void printToken(struct tokenType token);
