// Scintilla source code edit control
/** @file LexInno.cxx
 ** Lexer for Inno Setup scripts.
 **/
// Written by Friedrich Vedder <fvedd@t-online.de>, using code from LexOthers.cxx.
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "Platform.h"

#include "PropSet.h"
#include "Accessor.h"
#include "StyleContext.h"
#include "KeyWords.h"
#include "Scintilla.h"
#include "SciLexer.h"

static void ColouriseInnoDoc(unsigned int startPos, int length, int, WordList *keywordLists[], Accessor &styler) {
	int state = SCE_INNO_DEFAULT;
	char chPrev;
	char ch = 0;
	char chNext = styler[startPos];
	int lengthDoc = startPos + length;
	char *buffer = new char[length];
	int bufferCount = 0;
	bool isBOL, isEOL, isWS, isBOLWS = 0;

	WordList &sectionKeywords = *keywordLists[0];
	WordList &standardKeywords = *keywordLists[1];
	WordList &parameterKeywords = *keywordLists[2];
	WordList &preprocessorKeywords = *keywordLists[3];
	WordList &pascalKeywords = *keywordLists[4];
	WordList &userKeywords = *keywordLists[5];

	// Go through all provided text segment
	// using the hand-written state machine shown below
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	for (int i = startPos; i < lengthDoc; i++) {
		chPrev = ch;
		ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			i++;
			continue;
		}

		isBOL = (chPrev == 0) || (chPrev == '\n') || (chPrev == '\r' && ch != '\n');
		isBOLWS = (isBOL) ? 1 : (isBOLWS && (chPrev == ' ' || chPrev == '\t'));
		isEOL = (ch == '\n' || ch == '\r');
		isWS = (ch == ' ' || ch == '\t');

		switch(state) {
			case SCE_INNO_DEFAULT:
				if (ch == ';' && isBOLWS) {
					// Start of a comment
					state = SCE_INNO_COMMENT;
				} else if (ch == '[' && isBOLWS) {
					// Start of a section name
					bufferCount = 0;
					state = SCE_INNO_SECTION;
				} else if (ch == '#' && isBOLWS) {
					// Start of a preprocessor directive
					state = SCE_INNO_PREPROC;
				} else if (ch == '{' && chNext == '#') {
					// Start of a preprocessor inline directive
					state = SCE_INNO_PREPROC_INLINE;
				} else if ((ch == '{' && (chNext == ' ' || chNext == '\t'))
					   || (ch == '(' && chNext == '*')) {
					// Start of a Pascal comment
					state = SCE_INNO_COMMENT_PASCAL;
				} else if (ch == '"') {
					// Start of a double-quote string
					state = SCE_INNO_STRING_DOUBLE;
				} else if (ch == '\'') {
					// Start of a single-quote string
					state = SCE_INNO_STRING_SINGLE;
				} else if (isascii(ch) && (isalpha(ch) || (ch == '_'))) {
					// Start of an identifier
					bufferCount = 0;
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
					state = SCE_INNO_IDENTIFIER;
				} else {
					// Style it the default style
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

			case SCE_INNO_COMMENT:
				if (isEOL) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_COMMENT);
				}
				break;

			case SCE_INNO_IDENTIFIER:
				if (isascii(ch) && (isalnum(ch) || (ch == '_'))) {
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
				} else {
					state = SCE_INNO_DEFAULT;
					buffer[bufferCount] = '\0';

					// Check if the buffer contains a keyword
					if (standardKeywords.InList(buffer)) {
						styler.ColourTo(i-1,SCE_INNO_KEYWORD);
					} else if (parameterKeywords.InList(buffer)) {
						styler.ColourTo(i-1,SCE_INNO_PARAMETER);
					} else if (pascalKeywords.InList(buffer)) {
						styler.ColourTo(i-1,SCE_INNO_KEYWORD_PASCAL);
					} else if (userKeywords.InList(buffer)) {
						styler.ColourTo(i-1,SCE_INNO_KEYWORD_USER);
					} else {
						styler.ColourTo(i-1,SCE_INNO_DEFAULT);
					}

					// Push back the faulty character
					chNext = styler[i--];
					ch = chPrev;
				}
				break;

			case SCE_INNO_SECTION:
				if (ch == ']') {
					state = SCE_INNO_DEFAULT;
					buffer[bufferCount] = '\0';

					// Check if the buffer contains a section name
					if (sectionKeywords.InList(buffer)) {
						styler.ColourTo(i,SCE_INNO_SECTION);
					} else {
						styler.ColourTo(i,SCE_INNO_DEFAULT);
					}
				} else if (isascii(ch) && (isalnum(ch) || (ch == '_'))) {
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
				} else {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

			case SCE_INNO_PREPROC:
				if (isWS || isEOL) {
					if (isascii(chPrev) && isalpha(chPrev)) {
						state = SCE_INNO_DEFAULT;
						buffer[bufferCount] = '\0';

						// Check if the buffer contains a preprocessor directive
						if (preprocessorKeywords.InList(buffer)) {
							styler.ColourTo(i-1,SCE_INNO_PREPROC);
						} else {
							styler.ColourTo(i-1,SCE_INNO_DEFAULT);
						}

						// Push back the faulty character
						chNext = styler[i--];
						ch = chPrev;
					}
				} else if (isascii(ch) && isalpha(ch)) {
					if (chPrev == '#' || chPrev == ' ' || chPrev == '\t')
						bufferCount = 0;
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
				}
				break;

			case SCE_INNO_STRING_DOUBLE:
				if (ch == '"' || isEOL) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

			case SCE_INNO_STRING_SINGLE:
				if (ch == '\'' || isEOL) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

			case SCE_INNO_PREPROC_INLINE:
				if (ch == '}') {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_PREPROC_INLINE);
				} else if (isEOL) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

			case SCE_INNO_COMMENT_PASCAL:
				if (ch == '}' || (ch == ')' && chPrev == '*')) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_COMMENT_PASCAL);
				} else if (isEOL) {
					state = SCE_INNO_DEFAULT;
					styler.ColourTo(i,SCE_INNO_DEFAULT);
				}
				break;

		}
	}
	delete []buffer;
}

static const char * const innoWordListDesc[] = {
	"Sections",
	"Keywords",
	"Parameters",
	"Preprocessor directives",
	"Pascal keywords",
	"User defined keywords",
	0
};

static void FoldInnoDoc(unsigned int startPos, int length, int, WordList *[], Accessor &styler) {
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);

	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	bool headerPoint = false;
	int lev;

	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler[i+1];

		int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (style == SCE_INNO_SECTION)
			headerPoint = true;

		if (atEOL) {
			lev = SC_FOLDLEVELBASE;

			if (lineCurrent > 0) {
				int levelPrevious = styler.LevelAt(lineCurrent - 1);

				if (levelPrevious & SC_FOLDLEVELHEADERFLAG)
					lev = SC_FOLDLEVELBASE + 1;
				else
					lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
			}

			if (headerPoint)
				lev = SC_FOLDLEVELBASE;

			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;

			if (headerPoint)
				lev |= SC_FOLDLEVELHEADERFLAG;

			if (lev != styler.LevelAt(lineCurrent))
				styler.SetLevel(lineCurrent, lev);

			lineCurrent++;
			visibleChars = 0;
			headerPoint = false;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}

	if (lineCurrent > 0) {
		int levelPrevious = styler.LevelAt(lineCurrent - 1);

		if (levelPrevious & SC_FOLDLEVELHEADERFLAG)
			lev = SC_FOLDLEVELBASE + 1;
		else
			lev = levelPrevious & SC_FOLDLEVELNUMBERMASK;
	} else {
		lev = SC_FOLDLEVELBASE;
	}
	int flagsNext = styler.LevelAt(lineCurrent);
	styler.SetLevel(lineCurrent, lev | flagsNext & ~SC_FOLDLEVELNUMBERMASK);
}

LexerModule lmInno(SCLEX_INNOSETUP, ColouriseInnoDoc, "inno", FoldInnoDoc, innoWordListDesc);
