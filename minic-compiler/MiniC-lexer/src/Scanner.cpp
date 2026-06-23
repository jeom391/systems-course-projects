/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "Scanner.h"

extern FILE *sourceFile;                       // miniC source program

// 오류 상태 인식을 위한 전역 변수
int characterLiteralError;
int stringLiteralError;
int doubleLiteralError;


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);

// 함수 확장
void getNumberToken(char firstCharacter, struct tokenType *token);
char getCharacterLiteral();
void getStringLiteral(char str[]);
double getDotStartingDouble();
void printEscapedChar(char ch);
void printEscapedString(char str[]);


char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37         38        39                              */
	"while",
	// 확장 키워드와 리터럴 이름 추가
	"char",     "double",   "for",      "do",      "goto",    "switch",  "case",  "break", "default",
	"%char",    "%string",  "%double",
	"{",        "||",       "}"
};

char *keyword[NO_KEYWORD] = {
	"const",  "else",    "if",    "int",    "return",  "void",    "while",
	// 확장 키워드 문자열 추가
	"char",   "double",  "for",   "do",     "goto",    "switch",  "case",   "break",  "default"
};

enum tsymbol tnum[NO_KEYWORD] = {
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	// 확장 키워드 토큰 번호 추가
	tchar,     tdouble,   tfor,    tdo,      tgoto,     tswitch,   tcase,    tbreak,   tdefault
};

struct tokenType scanner()
{
	struct tokenType token;
	int i, index;
	int ch;	// EOF 처리를 위해 입력 문자를 int로 보관
	char id[ID_LENGTH];

	token.number = tnull;

	do {
		// 공백 문자는 토큰으로 만들지 않고 다음 문자를 읽음
		while (isspace(ch = fgetc(sourceFile)));	// state 1: skip blanks
		// 알파벳 또는 '_'로 시작하면 식별자나 키워드 후보로 인식
		if (superLetter(ch)) { // identifier or keyword
			i = 0;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0';
			ungetc(ch, sourceFile);  //  retract
									 // find the identifier in the keyword table
			// keyword 배열과 비교하여 예약어인지 일반 식별자인지 구분
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;
			if (index < NO_KEYWORD)    // found, keyword exit
				token.number = tnum[index];
			else {                     // not found, identifier exit
				token.number = tident;
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword
		else if (isdigit(ch)) {  // number
			// 숫자로 시작하면 정수 또는 실수 리터럴인지 getNumberToken()에서 판단
			doubleLiteralError = 0;	// 정수와 실수 리터럴을 함께 구분
			getNumberToken(ch, &token);
		}
		else switch (ch) {  // special character
		case '/':
			ch = fgetc(sourceFile);
			if (ch == '*')			// text comment
				do {
					while (ch != '*') ch = fgetc(sourceFile);
					ch = fgetc(sourceFile);
				} while (ch != '/');
			else if (ch == '/')		// line comment
				while (fgetc(sourceFile) != '\n');
			else if (ch == '=')  token.number = tdivAssign;
			else {
				token.number = tdiv;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '!':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tnotequ;
			else {
				token.number = tnot;
				ungetc(ch, sourceFile); // retract
			}
			break;
		case '%':
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				ungetc(ch, sourceFile);
			}
			break;
		case '&':
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '*':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '+':
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '-':
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '<':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '=':
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '>':
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '|':
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3);
				ungetc(ch, sourceFile);  // retract
			}
			break;
		case '(': token.number = tlparen;         break;
		case ')': token.number = trparen;         break;
		case ',': token.number = tcomma;          break;
		case ';': token.number = tsemicolon;      break;
		case '[': token.number = tlbracket;       break;
		case ']': token.number = trbracket;       break;
		case '\'':
			// 작은따옴표를 만나면 character literal 인식 함수로 넘김 (문자 리터럴 인식 확장)
			token.number = tcharLiteral;
			characterLiteralError = 0;
			token.value.ch = getCharacterLiteral();
			// 문자 리터럴 오류 시 tnull로 두어 scanner()가 다음 토큰 인식 이어감
			if (characterLiteralError) token.number = tnull;
			break;
		case '"':
			// 큰따옴표를 만나면 string literal 인식 함수로 넘김 (문자열 리터럴 인식 확장)
			token.number = tstringLiteral;
			stringLiteralError = 0;
			getStringLiteral(token.value.str);
			// 문자열 리터럴 오류 시 tnull로 두어 scanner()가 다음 토큰 인식 이어감
			if (stringLiteralError) token.number = tnull;
			break;
		case '.':
			// '.' 다음 문자가 digit이면 .123 형태의 실수 리터럴로 처리
			// .으로 시작하는 숏폼 인식
			ch = fgetc(sourceFile);
			if (isdigit(ch)) {
				ungetc(ch, sourceFile);
				token.number = tdoubleLiteral;
				doubleLiteralError = 0;
				token.value.dnum = getDotStartingDouble();
				if (doubleLiteralError) token.number = tnull;
			}
			else {
				if (ch != EOF) ungetc(ch, sourceFile);
				lexicalError(4);
			}
			break;
		case '{': token.number = tlbrace;         break;
		case '}': token.number = trbrace;         break;
		case EOF: token.number = teof;            break;
		default: {
			printf("Current character : %c", ch);
			lexicalError(4);
			break;
		}

		} // switch end
		// token.number가 tnull이면 정상 토큰이 나올 때까지 스캐너를 반복
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");
		break;
	case 2: printf("next character must be &\n");
		break;
	case 3: printf("next character must be |\n");
		break;
	case 4: printf("invalid character\n");
		break;
	// 문자 리터럴 오류 추가
	case 5: printf("invalid character literal\n");
		break;
	// 문자열 리터럴 오류 추가
	case 6: printf("invalid string literal\n");
		break;
	// 실수 리터럴 오류 추가
	case 7: printf("invalid double literal\n");
		break;
	// 리터럴 길이 오류 추가
	case 8: printf("literal length is too long\n");
		break;
	}
}

int superLetter(char ch)
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;
	else return 0;
}

int getNumber(char firstCharacter)
{
	int num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
	return num;
}

// 정수와 실수 리터럴을 구분하여 토큰 생성하도록 확장
void getNumberToken(char firstCharacter, struct tokenType *token)
{
	char buffer[STRING_LENGTH];
	char ch;
	int i = 0;
	int hasDot = 0;
	int hasExponent = 0;
	int valid = 1;

	// 숫자로 시작한 입력을 buffer에 모아 정수인지 실수인지 판단
	buffer[i++] = firstCharacter;
	ch = fgetc(sourceFile);

	// 0x 또는 0X는 기존 getNumber()를 이용해 16진수 정수로 처리
	if (firstCharacter == '0' && (ch == 'X' || ch == 'x')) {
		ungetc(ch, sourceFile);
		token->number = tnumber;
		token->value.num = getNumber(firstCharacter);
		return;
	}

	// 0으로 시작하면 먼저 8진수 정수 가능성을 처리
	if (firstCharacter == '0') {
		while (ch >= '0' && ch <= '7') {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
	}
	else {
		while (isdigit(ch)) {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
	}

	// '.'이 나오면 fixed-point 형태의 double literal로 판단
	if (ch == '.') {
		hasDot = 1;
		if (i < STRING_LENGTH - 1) buffer[i++] = ch;
		else lexicalError(8);
		ch = fgetc(sourceFile);
		while (isdigit(ch)) {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
	}

	// e 또는 E가 나오면 exponent 형태의 double literal로 판단
	if (ch == 'e' || ch == 'E') {
		hasExponent = 1;
		if (i < STRING_LENGTH - 1) buffer[i++] = ch;
		else lexicalError(8);
		ch = fgetc(sourceFile);
		if (ch == '+' || ch == '-') {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
		// exponent 뒤에는 반드시 digit이 와야 함
		if (!isdigit(ch)) valid = 0;
		while (isdigit(ch)) {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
	}

	buffer[i] = '\0';
	if (ch != EOF) ungetc(ch, sourceFile);

	if (hasDot || hasExponent) {
		if (!valid) {
			// e/E 뒤에 숫자가 없으면 잘못된 double literal로 처리
			lexicalError(7);
			doubleLiteralError = 1;
			token->number = tnull;
		}
		else {
			// 소수점 또는 exponent가 있으면 double literal 토큰 생성
			token->number = tdoubleLiteral;
			token->value.dnum = strtod(buffer, NULL);
		}
	}
	else {
		// 소수점과 exponent가 없으면 integer literal 토큰 생성
		int j;
		int num = 0;
		if (firstCharacter == '0') {
			for (j = 1; buffer[j] != '\0'; j++)
				num = 8 * num + (int)(buffer[j] - '0');
		}
		else {
			for (j = 0; buffer[j] != '\0'; j++)
				num = 10 * num + (int)(buffer[j] - '0');
		}
		token->number = tnumber;
		token->value.num = num;
	}
}

// 문자 리터럴 인식 함수 확장
char getCharacterLiteral()
{
	char ch, value, close;

	// 작은따옴표 이후 첫 문자를 읽고,
	ch = fgetc(sourceFile);
	// EOF 또는 줄바꿈이면 잘못된 character literal로 처리
	if (ch == EOF || ch == '\n') {
		lexicalError(5);
		characterLiteralError = 1;
		return '\0';
	}

	// backslash가 나오면 escape sequence로 처리
	if (ch == '\\') {
		ch = fgetc(sourceFile);
		switch (ch) {
		// 허용된 escape 문자만 정상 문자 값으로 변환
		case 'n': value = '\n'; break;
		case 't': value = '\t'; break;
		case '\\': value = '\\'; break;
		case '\'': value = '\''; break;
		case '"': value = '"'; break;
		default:
			lexicalError(5);
			while ((ch = fgetc(sourceFile)) != '\'' && ch != '\n' && ch != EOF);
			characterLiteralError = 1;
			return '\0';
		}
	}
	else value = ch;

	close = fgetc(sourceFile);
	if (close != '\'') {
		// 닫는 작은따옴표가 없으면 오류 처리 후 다음 토큰을 위해 입력 소비 처리
		lexicalError(5);
		while (close != '\'' && close != '\n' && close != EOF)
			close = fgetc(sourceFile);
		characterLiteralError = 1;
		return '\0';
	}

	// 정상적으로 인식된 문자 값을 반환
	return value;
}

// 문자열 리터럴 인식 함수 확장
void getStringLiteral(char str[])
{
	char ch;
	int i = 0;
	int valid = 1;

	// 닫는 큰따옴표를 만날 때까지 문자열 내용을 반복해서 읽음
	while ((ch = fgetc(sourceFile)) != '"' && ch != EOF && ch != '\n') {
		// backslash가 나오면 escape sequence로 처리
		if (ch == '\\') {
			ch = fgetc(sourceFile);
			// escape 처리 중 EOF 또는 줄바꿈을 만나면 잘못된 string literal
			if (ch == EOF || ch == '\n') {
				valid = 0;
				break;
			}
			switch (ch) {
			case 'n': ch = '\n'; break;
			case 't': ch = '\t'; break;
			case '\\': ch = '\\'; break;
			case '\'': ch = '\''; break;
			case '"': ch = '"'; break;
			default: break;
			}
		}
		// 문자열 저장 길이를 넘으면 길이 오류를 출력
		if (i < STRING_LENGTH - 1) str[i++] = ch;
		else lexicalError(8);
	}

	// 닫는 큰따옴표 없이 EOF 또는 줄바꿈을 만나면 오류로 처리
	if (ch != '"') valid = 0;
	str[i] = '\0';

	if (!valid) {
		// 오류가 발생하면 상태를 기록하고 문자열 값을 비움
		lexicalError(6);
		stringLiteralError = 1;
		str[0] = '\0';
	}
}

// 실수 리터럴 인식 함수 확장
double getDotStartingDouble()
{
	char buffer[STRING_LENGTH];
	char ch;
	int i = 0;
	int valid = 1;

	// '.'으로 시작하는 .123 형태의 double literal을 처리
	buffer[i++] = '.';
	ch = fgetc(sourceFile);
	// '.' 다음에 오는 digit들을 buffer에 저장
	while (isdigit(ch)) {
		if (i < STRING_LENGTH - 1) buffer[i++] = ch;
		else lexicalError(8);
		ch = fgetc(sourceFile);
	}

	// e 또는 E가 나오면 exponent 부분 처리
	if (ch == 'e' || ch == 'E') {
		if (i < STRING_LENGTH - 1) buffer[i++] = ch;
		else lexicalError(8);
		ch = fgetc(sourceFile);
		if (ch == '+' || ch == '-') {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
		// exponent 뒤에 숫자가 없으면 잘못된 double literal
		if (!isdigit(ch)) valid = 0;
		while (isdigit(ch)) {
			if (i < STRING_LENGTH - 1) buffer[i++] = ch;
			else lexicalError(8);
			ch = fgetc(sourceFile);
		}
	}

	buffer[i] = '\0';
	if (ch != EOF) ungetc(ch, sourceFile);

	if (!valid) {
		lexicalError(7);
		doubleLiteralError = 1;
		return 0.0;
	}
	// 정상적으로 인식되면 문자열을 double 값으로 변환
	return strtod(buffer, NULL);
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

// 문자 리터럴 출력 함수 확장
void printEscapedChar(char ch)
{
	switch (ch) {
	case '\n': printf("\\n"); break;
	case '\t': printf("\\t"); break;
	case '\\': printf("\\\\"); break;
	case '\'': printf("\\'"); break;
	case '"': printf("\\\""); break;
	default: printf("%c", ch); break;
	}
}

// 문자열 리터럴 출력 함수 확장
void printEscapedString(char str[])
{
	int i;

	for (i = 0; str[i] != '\0'; i++)
		printEscapedChar(str[i]);
}

void printToken(struct tokenType token)
{
	// 식별자와 리터럴 토큰은 token value를 함께 출력
	if (token.number == tident)
		printf("number: %d, value: %s\n", token.number, token.value.id);
	else if (token.number == tnumber)
		printf("number: %d, value: %d\n", token.number, token.value.num);
	else if (token.number == tcharLiteral) {
		// 문자 리터럴은 escape 문자가 화면에 보이도록 출력
		printf("number: %d, value: '", token.number);
		printEscapedChar(token.value.ch);
		printf("'\n");
	}
	else if (token.number == tstringLiteral) {
		// 문자열 리터럴은 escape 문자가 화면에 보이도록 출력
		printf("number: %d, value: \"", token.number);
		printEscapedString(token.value.str);
		printf("\"\n");
	}
	else if (token.number == tdoubleLiteral)
		printf("number: %d, value: %lf\n", token.number, token.value.dnum);
	else
		// 그 외 토큰은 tokenName[] 배열의 이름을 출력
		printf("number: %d(%s)\n", token.number, tokenName[token.number]);

}
