/* 

  SjASMPlus Z80 Cross Compiler

  This is modified source of SjASM by Aprisobal - aprisobal@tut.by

  Copyright (c) 2006 Sjoerd Mastijn

  This software is provided 'as-is', without any express or implied warranty.
  In no event will the authors be held liable for any damages arising from the
  use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it freely,
  subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
	 that you wrote the original software. If you use this software in a product,
	 an acknowledgment in the product documentation would be appreciated but is
	 not required.

  2. Altered source versions must be plainly marked as such, and must not be
	 misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.

*/

#include "global.h"
#include "parser.h"
#include "reader.h"

bool cmphstr(const char *&p1, const char *p2, bool AllowParen) {
    /* old:
    if (isupper(*p1))
      while (p2[i]) {
        if (p1[i]!=toupper(p2[i])) return 0;
        ++i;
      }
    else
      while (p2[i]) {
        if (p1[i]!=p2[i]) return 0;
        ++i;
      }*/
    if (strlen(p1) >= strlen(p2)) {
        unsigned int i = 0;
        unsigned int v = 0;
        auto optUpper = isupper(*p1) ?
                        std::function<char(char)>([](char C) { return toupper(C); }) :
                        std::function<char(char)>([](char C) { return C; });
        while (p2[i]) {
            if (p1[i] != optUpper(p2[i])) {
                v = 0;
            } else {
                ++v;
            }
            ++i;
        }
        if (strlen(p2) != v) {
            return false;
        }

        if (i <= strlen(p1) && p1[i] > ' ' && !(AllowParen && p1[i] == '(')/* && p1[i]!=':'*/) {
            return false;
        }
        p1 += i;
        return true;
    } else {
        return false;
    }
}

bool White(char c) {
    return isspace(c);
}

bool White() {
    return White(*lp);
}

void SkipBlanks(const char *&p) {
    while ((*p > 0) && (*p <= ' ')) {
        ++p;
    }
}

bool SkipBlanks() {
    SkipBlanks(lp);
    return (*lp == 0);
}

/* added */
void SkipParam(const char *&p) {
    SkipBlanks(p);
    if (!(*p)) {
        return;
    }
    while (((*p) != '\0') && ((*p) != ',')) {
        p++;
    }
}

bool NeedEQU() {
    const char *olp = lp;
    SkipBlanks();
    /*if (*lp=='=') { ++lp; return 1; }*/
    /* cut: if (*lp=='=') { ++lp; return 1; } */
    if (*lp == '.') {
        ++lp;
    }
    if (cmphstr(lp, "equ")) {
        return true;
    }
    lp = olp;
    return false;
}

/* added */
bool NeedDEFL() {
    const char *olp = lp;
    SkipBlanks();
    if (*lp == '=') {
        ++lp;
        return true;
    }
    if (*lp == '.') {
        ++lp;
    }
    if (cmphstr(lp, "defl")) {
        return true;
    }
    lp = olp;
    return false;
}

bool comma(const char *&p) {
    SkipBlanks(p);
    if (*p != ',') {
        return false;
    }
    ++p;
    return true;
}

int cpc = '4';

/* not modified */
bool oparen(const char *&p, char c) {
    SkipBlanks(p);
    if (*p != c) {
        return false;
    }
    if (c == '[') {
        cpc = ']';
    }
    if (c == '(') {
        cpc = ')';
    }
    if (c == '{') {
        cpc = '}';
    }
    ++p;
    return true;
}

bool cparen(const char *&p) {
    SkipBlanks(p);
    if (*p != cpc) {
        return false;
    }
    ++p;
    return true;
}

const char * getparen(const char *p) {
    int teller = 0;
    SkipBlanks(p);
    while (*p) {
        if (*p == '(') {
            ++teller;
        } else if (*p == ')') {
            if (teller == 1) {
                SkipBlanks(++p);
                return p;
            } else {
                --teller;
            }
        }
        ++p;
    }
    return nullptr;
}

optional<std::string> getID(const char *&p) {
    std::string S;
    SkipBlanks(p);
    //if (!isalpha(*p) && *p!='_') return 0;
    if (*p && !isalpha((unsigned char) *p) && *p != '_') {
        return boost::none;
    }
    while (*p) {
        if (!isalnum((unsigned char) *p) && *p != '_' && *p != '.' && *p != '?' && *p != '!' && *p != '#' &&
            *p != '@') {
            break;
        }
        S.push_back(*p);
        ++p;
    }
    if (!S.empty()) return S;
    else return boost::none;
}

std::string getInstr(const char *&p) {
    std::string I;
    SkipBlanks(p);
    if (!isalpha((unsigned char) *p) && *p != '.') {
        return ""s;
    } else {
        I += *p;
        ++p;
    }
    while (*p) {
        if (!isalnum((unsigned char) *p) && *p != '_') {
            break;
        }
        I += *p;
        ++p;
    }
    return I;
}

/* changes applied from SjASM 0.39g */
bool check8(aint val, bool error) {
    if (val != (val & 255) && ~val > 127 && error) {
        Error("Value doesn't fit into 8 bits"s, std::to_string(val));
        return false;
    }
    return true;
}

/* changes applied from SjASM 0.39g */
bool check8o(long val) {
    if (val < -128 || val > 127) {
        Error("check8o(): Offset out of range"s, std::to_string(val));
        return false;
    }
    return true;
}

/* changes applied from SjASM 0.39g */
bool check16(aint val, bool error) {
    if (val != (val & 65535) && ~val > 32767 && error) {
        Error("Value does not fit into 16 bits"s, std::to_string(val));
        return false;
    }
    return true;
}

/* changes applied from SjASM 0.39g */
bool check24(aint val, bool error) {
    if (val != (val & 16777215) && ~val > 8388607 && error) {
        Error("Value does not fit into 24 bits"s, std::to_string(val));
        return false;
    }
    return true;
}

bool need(const char *&p, char c) {
    SkipBlanks(p);
    if (*p != c) {
        return false;
    }
    ++p;
    return true;
}

int needa(const char *&p,
          const char *c1, int r1,
          const char *c2, int r2,
          const char *c3, int r3,
          bool AllowParen) {
    //  SkipBlanks(p);
    if (!isalpha((unsigned char) *p)) {
        return 0;
    }
    if (cmphstr(p, c1, AllowParen)) {
        return r1;
    }
    if (c2 && cmphstr(p, c2, AllowParen)) {
        return r2;
    }
    if (c3 && cmphstr(p, c3, AllowParen)) {
        return r3;
    }
    return 0;
}

int need(const char *&p, const char *c) {
    SkipBlanks(p);
    while (*c) {
        if (*p != *c) {
            c += 2;
            continue;
        }
        ++c;
        if (*c == ' ') {
            ++p;
            return *(c - 1);
        }
        if (*c == '_' && *(p + 1) != *(c - 1)) {
            ++p;
            return *(c - 1);
        }
        if (*(p + 1) == *c) {
            p += 2;
            return *(c - 1) + *c;
        }
        ++c;
    }
    return 0;
}

int getval(int p) {
    switch (p) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return p - '0';
        default:
            if (isupper((unsigned char) p)) {
                return p - 'A' + 10;
            }
            if (islower((unsigned char) p)) {
                return p - 'a' + 10;
            }
            return 200;
    }
}

bool GetConstant(const char *&op, aint &val) {
    aint base, pb = 1, v, oval;
    const char *p = op, *p2, *p3;

    SkipBlanks(p);

    p3 = p;
    val = 0;

    switch (*p) {
        case '#':
        case '$':
            ++p;
            while (isalnum((unsigned char) *p)) {
                if ((v = getval(*p)) >= 16) {
                    Error("Digit not in base"s, op);
                    return false;
                }
                oval = val;
                val = val * 16 + v;
                ++p;
                if (oval > val) {
                    Error("Overflow"s, SUPPRESS);
                }
            }

            if (p - p3 < 2) {
                Error("Syntax error"s, op, CATCHALL);
                return false;
            }

            op = p;

            return true;
        case '%':
            ++p;
            while (isdigit((unsigned char) *p)) {
                if ((v = getval(*p)) >= 2) {
                    Error("Digit not in base"s, op);
                    return false;
                }
                oval = val;
                val = val * 2 + v;
                ++p;
                if (oval > val) {
                    Error("Overflow"s, SUPPRESS);
                }
            }
            if (p - p3 < 2) {
                Error("Syntax error"s, op, CATCHALL);
                return false;
            }

            op = p;

            return true;
        case '0':
            ++p;
            if (*p == 'x' || *p == 'X') {
                ++p;
                while (isalnum((unsigned char) *p)) {
                    if ((v = getval(*p)) >= 16) {
                        Error("Digit not in base"s, op);
                        return false;
                    }
                    oval = val;
                    val = val * 16 + v;
                    ++p;
                    if (oval > val) {
                        Error("Overflow"s, SUPPRESS);
                    }
                }
                if (p - p3 < 3) {
                    Error("Syntax error"s, op, CATCHALL);
                    return false;
                }

                op = p;

                return true;
            }
        default:
            while (isalnum((unsigned char) *p)) {
                ++p;
            }
            p2 = p--;
            if (isdigit((unsigned char) *p)) {
                base = 10;
            } else if (*p == 'b') {
                base = 2;
                --p;
            } else if (*p == 'h') {
                base = 16;
                --p;
            } else if (*p == 'B') {
                base = 2;
                --p;
            } else if (*p == 'H') {
                base = 16;
                --p;
            } else if (*p == 'o') {
                base = 8;
                --p;
            } else if (*p == 'q') {
                base = 8;
                --p;
            } else if (*p == 'd') {
                base = 10;
                --p;
            } else if (*p == 'O') {
                base = 8;
                --p;
            } else if (*p == 'Q') {
                base = 8;
                --p;
            } else if (*p == 'D') {
                base = 10;
                --p;
            } else {
                return false;
            }
            do {
                if ((v = getval(*p)) >= base) {
                    Error("Digit not in base"s, op);
                    return false;
                }
                oval = val;
                val += v * pb;
                if (oval > val) {
                    Error("Overflow"s, SUPPRESS);
                }
                pb *= base;
            } while (p-- != p3);

            op = p2;

            return true;
    }
}

bool GetCharConstChar(const char *&op, aint &val) {
    if ((val = *op++) != '\\') {
        return true;
    }
    switch (val = *op++) {
        case '\\':
        case '\'':
        case '\"':
        case '\?':
            return true;
        case 'n':
        case 'N':
            val = 10;
            return true;
        case 't':
        case 'T':
            val = 9;
            return true;
        case 'v':
        case 'V':
            val = 11;
            return true;
        case 'b':
        case 'B':
            val = 8;
            return true;
        case 'r':
        case 'R':
            val = 13;
            return true;
        case 'f':
        case 'F':
            val = 12;
            return true;
        case 'a':
        case 'A':
            val = 7;
            return true;
        case 'e':
        case 'E':
            val = 27;
            return true;
        case 'd':
        case 'D':
            val = 127;
            return true;
        default:
            --op;
            val = '\\';

            Error("Unknown escape"s, op);

            return true;
    }
    return false;
}

/* added */
bool GetCharConstCharSingle(const char *&op, aint &val) {
    if ((val = *op++) != '\\') {
        return true;
    }
    switch (val = *op++) {
        case '\'':
            return true;
    }
    --op;
    val = '\\';
    return true;
}

bool GetCharConst(const char *&p, aint &val) {
    aint s = 24, r, t = 0;
    val = 0;
    const char *op = p;
    char q;
    if (*p != '\'' && *p != '"') {
        return false;
    }
    q = *p++;
    do {
        if (!*p || *p == q) {
            p = op;
            return false;
        }
        GetCharConstChar(p, r);
        val += r << s;
        s -= 8;
        ++t;
    } while (*p != q);
    if (t > 4) {
        Error("Overflow"s, SUPPRESS);
    }
    val = val >> (s + 8);
    ++p;
    return true;
}

/* modified */
int GetBytes(const char *&p, int *e, int add, int dc) {
    aint val;
    int t = 0;
    while (true) {
        SkipBlanks(p);
        if (!*p) {
            Error("Expression expected"s, SUPPRESS);
            break;
        }
        if (t == 128) {
            Error("Too many arguments"s, p, SUPPRESS);
            break;
        }
        if (*p == '"') {
            p++;
            do {
                if (!*p || *p == '"') {
                    Error("Syntax error"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                if (t == 128) {
                    Error("Too many arguments"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                GetCharConstChar(p, val);
                check8(val);
                e[t++] = (val + add) & 255;
            } while (*p != '"');
            ++p;
            if (dc && t) {
                e[t - 1] |= 128;
            }
            /* (begin add) */
        } else if ((*p == 0x27) && (!*(p + 2) || *(p + 2) != 0x27)) {
            p++;
            do {
                if (!*p || *p == 0x27) {
                    Error("Syntax error"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                if (t == 128) {
                    Error("Too many arguments"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                GetCharConstCharSingle(p, val);
                check8(val);
                e[t++] = (val + add) & 255;
            } while (*p != 0x27);

            ++p;

            if (dc && t) {
                e[t - 1] |= 128;
            }
            /* (end add) */
        } else {
            if (parseExpression(p, val)) {
                check8(val);
                e[t++] = (val + add) & 255;
            } else {
                Error("Syntax error"s, p, SUPPRESS);
                break;
            }
        }
        SkipBlanks(p);
        if (*p != ',') {
            break;
        }
        ++p;
    }
    e[t] = -1;
    return t;
}

std::string getString(const char *&p, bool KeepBrackets) {
    std::string Res;
    SkipBlanks(p);
    if (!*p) {
        return std::string{};
    }
    char limiter = '\0';

    if (*p == '"') {
        limiter = '"';
        ++p;
    } else if (*p == '<') {
        limiter = '>';
        if (KeepBrackets)
            Res += *p;
        ++p;
    }
    //TODO: research strange ':' logic
    while (*p && *p != limiter) {
        Res += *p++;
    }
    if (*p != limiter) {
        Error("No closing '"s + std::string{limiter} + "'"s);
    }
    if (*p) {
        if (*p == '>' && KeepBrackets)
            Res += *p;
        ++p;
    }
    return Res;
}

fs::path getFileName(const char *&p) {
    const std::string &result = getString(p, true);
    return fs::path(result);
}

HobetaFilename GetHobetaFileName(const char *&p) {
    const std::string &result = getString(p);
    return HobetaFilename(result);
}

bool needcomma(const char *&p) {
    SkipBlanks(p);
    if (*p != ',') {
        Error("Comma expected"s);
    }
    return (*(p++) == ',');
}

bool needbparen(const char *&p) {
    SkipBlanks(p);
    if (*p != ']') {
        Error("']' expected"s);
    }
    return (*(p++) == ']');
}

bool islabchar(char p) {
    if (isalnum((unsigned char) p) || p == '_' || p == '.' || p == '?' || p == '!' || p == '#' || p == '@') {
        return true;
    }
    return false;
}

/* added */
int GetArray(const char *&p, int *e, int add, int dc) {
    aint val;
    int t = 0;
    while (true) {
        SkipBlanks(p);
        if (!*p) {
            Error("Expression expected"s, SUPPRESS);
            break;
        }
        if (t == 128) {
            Error("Too many arguments"s, p, SUPPRESS);
            break;
        }
        if (*p == '"') {
            p++;
            do {
                if (!*p || *p == '"') {
                    Error("Syntax error"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                if (t == 128) {
                    Error("Too many arguments"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                GetCharConstChar(p, val);
                check8(val);
                e[t++] = (val + add) & 255;
            } while (*p != '"');
            ++p;
            if (dc && t) {
                e[t - 1] |= 128;
            }
            /* (begin add) */
        } else if ((*p == 0x27) && (!*(p + 2) || *(p + 2) != 0x27)) {
            p++;
            do {
                if (!*p || *p == 0x27) {
                    Error("Syntax error"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                if (t == 128) {
                    Error("Too many arguments"s, p, SUPPRESS);
                    e[t] = -1;
                    return t;
                }
                GetCharConstCharSingle(p, val);
                check8(val);
                e[t++] = (val + add) & 255;
            } while (*p != 0x27);
            ++p;
            if (dc && t) {
                e[t - 1] |= 128;
            }
            /* (end add) */
        } else {
            if (parseExpression(p, val)) {
                check8(val);
                e[t++] = (val + add) & 255;
            } else {
                Error("Syntax error"s, p, SUPPRESS);
                break;
            }
        }
        SkipBlanks(p);
        if (*p != ',') {
            break;
        }
        ++p;
    }
    e[t] = -1;
    return t;
}

const std::string getAll(const char *&p) {
    std::string R{p};
    while ((*p > 0)) {
        ++p;
    }
    return R;
}

//eof reader.cpp
