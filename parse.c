#include "parse.h"
#include "asminfo.h"

char *input_path, *output_path;
int current_line;
char current_error[500];
char current_warning[500];

int bin_str_to_num(char *ch, int *num) {
	int baseline = 0, sign = 1, ans = 0, temp;
	int i, j, length = strlen(ch);

	assert(ch);
	assert(num);

	switch (*ch) {
	case '+': baseline = 1; sign = 1; break;
	case '-': baseline = 1; sign = -1; break;
	}

	for (i = length - 1; i >= baseline; i--) {
		if (ch[i] == '0' || ch[i] == '1') {
			temp = ch[i] - 48;
			for (j = 0; j < length - 1 - i; j++) {
				temp *= 2;
			}
			ans += temp;
		}
		else {
			sprintf(current_error, "illegal number character \'%c\' in a binary immediate value", ch[i]);
			PRINT_ERROR(current_error);
			return 0;
		}
	}

	*num = ans * sign;
	return 1;
}

int oct_str_to_num(char *ch, int *num) {
	int baseline = 0, sign = 1, ans = 0, temp;
	int i, j, length = strlen(ch);

	assert(ch);
	assert(num);

	switch (*ch) {
	case '+': baseline = 1; sign = 1; break;
	case '-': baseline = 1; sign = -1; break;
	}

	for (i = length - 1; i >= baseline; i--) {
		if (ch[i] >= '0' && ch[i] <= '7') {
			temp = ch[i] - 48;
			for (j = 0; j < length - 1 - i; j++) {
				temp *= 8;
			}
			ans += temp;
		}
		else {
			sprintf(current_error, "illegal number character \'%c\' in an octal immediate value", ch[i]);
			PRINT_ERROR(current_error);
			return 0;
		}
	}

	*num = ans * sign;
	return 1;
}

int dec_str_to_num(char *ch, int *num) {
	int baseline = 0, sign = 1, ans = 0, temp;
	int i, j, length = strlen(ch);

	assert(ch);
	assert(num);

	switch (*ch) {
	case '+': baseline = 1; sign = 1; break;
	case '-': baseline = 1; sign = -1; break;
	}

	for (i = length - 1; i >= baseline; i--) {
		if (ch[i] >= '0' && ch[i] <= '9') {
			temp = ch[i] - 48;
			for (j = 0; j < length - 1 - i; j++) {
				temp *= 10;
			}
			ans += temp;
		}
		else {
			sprintf(current_error, "illegal number character \'%c\' in a decimal immediate value", ch[i]);
			PRINT_ERROR(current_error);
			return 0;
		}
	}

	*num = ans * sign;
	return 1;
}

int hex_str_to_num(char *ch, int *num) {
	int baseline = 0, sign = 1, ans = 0, temp;
	int i, j, length = strlen(ch);

	assert(ch);
	assert(num);

	switch (*ch) {
	case '+': baseline = 1; sign = 1; break;
	case '-': baseline = 1; sign = -1; break;
	}

	for (i = length - 1; i >= baseline; i--) {
		if ((ch[i] >= '0' && ch[i] <= '9') || (ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F')) {
			if (ch[i] >= 'A' && ch[i] <= 'F')
				temp = ch[i] - 55;
			else if (ch[i] > 'a' && ch[i] <= 'f')
				temp = ch[i] - 87;
			else
				temp = ch[i] - 48;
			for (j = 0; j < length - 1 - i; j++) {
				temp *= 16;
			}
			ans += temp;

		}
		else {
			sprintf(current_error, "illegal number character \'%c\' in an hexadecimal immediate value", ch[i]);
			PRINT_ERROR(current_error);
			return 0;
		}
	}

	*num = ans * sign;
	return 1;
}

int str_to_reg(char *ch, Byte *reg) {
	int ans;

	assert(ch);
	assert(reg);

	if (!IS_REG_PREFIX(ch[0])) {
		sprintf(current_error, "not a register prefix \'r\' or \'R\'");
		PRINT_ERROR(current_error);
		return 0;
	}

	if (dec_str_to_num(ch + 1, &ans) == 0) {
		return 0;
	}

	if (ans < 1 || ans > 32) {
		sprintf(current_error, "register %d does not exist", ans);
		PRINT_ERROR(current_error);
		return 0;
	}

	*reg = (Byte)ans;
	return 1;
}

int escape_char_to_char(char *ch, char *res) {
    int state = 0, i, mcwflag = 0;
    char str[100], *str_p = str;
    char res_buff[100], *res_p = res_buff;
    int num;

    assert(ch);
    assert(res);

    for (i = 0; ; i++) {
        printf("char_state = %d\n", state);
        switch (state) {
            case 0:
                if (i && !mcwflag) {
                    sprintf(current_warning, "multi-character character constant");
                    PRINT_WARNING(current_warning);
                    mcwflag = 1;
                }
                str_p = str;
                if (IS_BS(ch[i])) {
                    state = 1;
                } else if (IS_TERMINATOR(ch[i])) {
                    *res = *(res_p - 1);
                    return 1;
                } else {
                    *(res_p++) = ch[i];
                }
                break;

            case 1:
                switch (ch[i]) {
                    case 'a': *(res_p++) = 7; state = 0; break;
                    case 'b': *(res_p++) = 8; state = 0; break;
                    case 'f': *(res_p++) = 12; state = 0; break;
                    case 'n': *(res_p++) = 10; state = 0; break;
                    case 'r': *(res_p++) = 13; state = 0; break;
                    case 't': *(res_p++) = 9; state = 0; break;
                    case '\\': *(res_p++) = 92; state = 0; break;
                    case '?': *(res_p++) = 63; state = 0; break;
                    case '\'': *(res_p++) = 39; state = 0; break;
                    case '\"': *(res_p++) = 34; state = 0; break;
                    default:
                        if (ch[i] == '0') {
                            *(str_p++) = ch[i];
                            state = 10;
                        }
                        else if (ch[i] >= '1' && ch[i] <= '7') {
                            *(str_p++) = ch[i];
                            state = 20;
                        }
                        else if (ch[i] == 'x') {
                            state = 30;
                        }
                        else {
                            sprintf(current_warning, "unknown escape sequence \'\\%c\'", ch[i]);
                            PRINT_WARNING(current_warning);
                            *(res_p++) = ch[i];
                            state = 0;
                        }
                        break;
                }
                break;

            case 10:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 21;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 20:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 21;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *(res_p++) = str[0];
                    *res = *(res_p - 1);
                    return 0;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 21:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 22;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 22:
                if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 30:
                if ((ch[i] >= '0' && ch[i] <= '9') || (ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F')) {
                    *(str_p++) = ch[i];
                    state = 31;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *(res_p++) = ch[i - 1];
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 31:
                if ((ch[i] >= '0' && ch[i] <= '9') || (ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F')) {
                    *(str_p++) = ch[i];
                    state = 32;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 32:
                if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res = *(res_p - 1);
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;
        }
    }
}

int escape_string_to_string(char *ch, char *res) {
    int state = 0, i;
    char str[100], *str_p = str, *res_p = res;
    int num;

    assert(ch);
    assert(res);

    for (i = 0; ; i++) {
        printf("char_state = %d\n", state);
        switch (state) {
            case 0:
                str_p = str;
                if (IS_BS(ch[i])) {
                    state = 1;
                } else if (IS_TERMINATOR(ch[i])) {
                    *res_p = 0;
                    return 1;
                } else {
                    *(res_p++) = ch[i];
                }
                break;

            case 1:
                switch (ch[i]) {
                    case 'a': *(res_p++) = 7; state = 0; break;
                    case 'b': *(res_p++) = 8; state = 0; break;
                    case 'f': *(res_p++) = 12; state = 0; break;
                    case 'n': *(res_p++) = 10; state = 0; break;
                    case 'r': *(res_p++) = 13; state = 0; break;
                    case 't': *(res_p++) = 9; state = 0; break;
                    case '\\': *(res_p++) = 92; state = 0; break;
                    case '?': *(res_p++) = 63; state = 0; break;
                    case '\'': *(res_p++) = 39; state = 0; break;
                    case '\"': *(res_p++) = 34; state = 0; break;
                    default:
                        if (ch[i] == '0') {
                            *(str_p++) = ch[i];
                            state = 10;
                        }
                        else if (ch[i] >= '1' && ch[i] <= '7') {
                            *(str_p++) = ch[i];
                            state = 20;
                        }
                        else if (ch[i] == 'x') {
                            state = 30;
                        }
                        else {
                            sprintf(current_warning, "unknown escape sequence \'\\%c\'", ch[i]);
                            PRINT_WARNING(current_warning);
                            *(res_p++) = ch[i];
                            state = 0;
                        }
                        break;
                }
                break;

            case 10:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 21;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *res_p = 0;
                    return 1;
                }
                else {
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 20:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 21;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *(res_p++) = str[0];
                    *res_p = 0;
                    return 0;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 21:
                if (ch[i] >= '0' && ch[i] <= '7') {
                    *(str_p++) = ch[i];
                    state = 22;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res_p = 0;
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 22:
                if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res_p = 0;
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (oct_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 30:
                if ((ch[i] >= '0' && ch[i] <= '9') || (ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F')) {
                    *(str_p++) = ch[i];
                    state = 31;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *(res_p++) = ch[i - 1];
                    *res_p = 0;
                    return 1;
                }
                else {
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 31:
                if ((ch[i] >= '0' && ch[i] <= '9') || (ch[i] >= 'a' && ch[i] <= 'f') || (ch[i] >= 'A' && ch[i] <= 'F')) {
                    *(str_p++) = ch[i];
                    state = 32;
                }
                else if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res_p = 0;
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;

            case 32:
                if (IS_TERMINATOR(ch[i])) {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *res_p = 0;
                    return 1;
                }
                else {
                    *str_p = 0;
                    if (hex_str_to_num(str, &num) == 0)
                        return 0;
                    *(res_p++) = (char)num;
                    *(res_p++) = ch[i];
                    state = 0;
                }
                break;
        }
    }
}

int parse_arg(char *ch, Arg_info *arginfo) {
	int state = 0, i;
	char arg_buff[500], *arg_p = arg_buff;
	int num;
	Byte reg;

	assert(ch);
	assert(arginfo);

	for (i = 0; ; i++) {
		printf("arg_state = %d\n", state);
		switch (state) {
		case 0:
			if (IS_NUM(ch[i]) || IS_PLUS(ch[i]) || IS_MINUS(ch[i])) {
				*(arg_p++) = ch[i];
				state = 1;
			}
			else if (IS_REG_PREFIX(ch[i])) {
				*(arg_p++) = ch[i];
				state = 3;
			}
			else if (IS_SQ(ch[i])) {
				state = 10;
			}
			else if (IS_DQ(ch[i])) {
				state = 30;
			}
			else if (IS_SPACE(ch[i])) {
			}
			else if (IS_UNDERLINE(ch[i]) || IS_ALPHA(ch[i]) || IS_DOT(ch[i])) {
				*(arg_p++) = ch[i];
				state = 20;
			}
			else if (IS_TERMINATOR(ch[i])) {
				sprintf(current_error, "empty argument, the character \',\' must be followed by an argument");
				PRINT_ERROR(current_error);
				return 0;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 1:
			if (IS_NUM(ch[i])) {
				*(arg_p++) = ch[i];
			}
			else if (IS_B_SUFFIX(ch[i])) {
				*arg_p = 0;
				if (bin_str_to_num(arg_buff, &num) == 0)
					return 0;
				state = 2;
			}
			else if (IS_O_SUFFIX(ch[i])) {
				*arg_p = 0;
				if (oct_str_to_num(arg_buff, &num) == 0)
					return 0;
				state = 2;
			}
			else if (IS_D_SUFFIX(ch[i])) {
				*arg_p = 0;
				if (dec_str_to_num(arg_buff, &num) == 0)
					return 0;
				state = 2;
			}
			else if (IS_H_SUFFIX(ch[i])) {
				*arg_p = 0;
				if (hex_str_to_num(arg_buff, &num) == 0)
					return 0;
				state = 2;
			}
			else if (IS_SPACE(ch[i])) {
				*arg_p = 0;
				if (dec_str_to_num(arg_buff, &num) == 0)
					return 0;
				state = 2;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				if (dec_str_to_num(arg_buff, &num) == 0)
					return 0;
				arginfo->type = AT_NUM;
				arginfo->u.num = num;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 2:
			if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
				arginfo->type = AT_NUM;
				arginfo->u.num = num;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 3:
			if (IS_NUM(ch[i])) {
				*(arg_p++) = ch[i];
				state = 4;
			}
			else if (IS_SPACE(ch[i])) {
			}
			else if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i])) {
				*(arg_p++) = ch[i];
				state = 21;
			}
			else if (IS_TERMINATOR(ch[i])) {
				sprintf(current_error, "register prefix \'%c\' must be followed by a index number", ch[i - 1]);
				PRINT_ERROR(current_error);
				return 0;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 4:
			if (IS_NUM(ch[i])) {
				*(arg_p++) = ch[i];
			}
			else if (IS_SPACE(ch[i])) {
				*arg_p = 0;
				if (str_to_reg(arg_buff, &reg) == 0)
					return 0;
				state = 5;
			}
			else if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i])) {
				*(arg_p++) = ch[i];
				state = 21;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				if (str_to_reg(arg_buff, &reg) == 0)
					return 0;
				arginfo->type = AT_REGISTER;
				arginfo->u.reg = reg;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 5:
			if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
				arginfo->type = AT_REGISTER;
				arginfo->u.reg = reg;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 10:
			if (IS_SQ(ch[i])) {
				sprintf(current_error, "illegal empty character constant");
				PRINT_ERROR(current_error);
				return 0;
			}
			else if (IS_TERMINATOR(ch[i])) {
				sprintf(current_error, "character constant must be ended with single quotation \"'\"");
				PRINT_ERROR(current_error);
				return 0;
			}
			else {
				*(arg_p++) = ch[i];
				state = 11;
			}
			break;

		case 11:
			if (IS_SQ(ch[i])) {
				state = 12;
			}
			else if (IS_TERMINATOR(ch[i])) {
				sprintf(current_error, "character constant must be ended with single quotation \"'\"");
				PRINT_ERROR(current_error);
				return 0;
			}
			else {
                *(arg_p++) = ch[i];
			}
			break;

		case 12:
			if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				arginfo->type = AT_NUM;
                if (escape_char_to_char(arg_buff, &(arginfo->u.num)) == 0)
                    return 0;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 20:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_NUM(ch[i])) {
				*(arg_p++) = ch[i];
				state = 21;
			}
			else if (IS_SPACE(ch[i])) {
				state = 22;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				arginfo->type = AT_LABEL_NAME;
				strcpy(arginfo->u.label_name, arg_buff);
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 21:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_NUM(ch[i])) {
				*(arg_p++) = ch[i];
			}
			else if (IS_SPACE(ch[i])) {
				state = 22;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				arginfo->type = AT_LABEL_NAME;
				strcpy(arginfo->u.label_name, arg_buff);
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 22:
			if (IS_SPACE(ch[i])) {

			}
			else if (IS_TERMINATOR(ch[i])) {
				*arg_p = 0;
				arginfo->type = AT_LABEL_NAME;
				strcpy(arginfo->u.label_name, arg_buff);
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

			case 30:
				if (IS_TERMINATOR(ch[i])) {
					sprintf(current_error, "string constant must be ended with double quotation \'\"\'");
					PRINT_ERROR(current_error);
					return 0;
				}
				else if (IS_DQ(ch[i])) {
					*arg_p = 0;
					arginfo->type = AT_STRING;
					if (escape_string_to_string(arg_buff, arginfo->u.string) == 0)
					    return 0;
					state = 31;
				}
				else {
					*(arg_p++) = ch[i];
				}
				break;

			case 31:
				if (IS_TERMINATOR(ch[i])) {
					return 1;
				}
				else if (IS_SPACE(ch[i])) {

				}
				else {
					sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
					PRINT_ERROR(current_error);
					return 0;
				}
				break;
		}
	}
}

int parse_line(char *ch, Row_info *row) {
	int state = 0, i;
	char arg_buff[100], *arg_p = arg_buff;
	char label_or_inst_buff[100], *label_or_inst_p = label_or_inst_buff;
	char inst_buff[100], *inst_p = inst_buff;
	int argnum = 0;

	assert(ch);
	assert(row);

	// fill row struct with zero.
	memset(row, 0, sizeof(*row));

	for (i = 0; ; i++) {
		printf("line_state = %d\n", state);
		switch (state) {
		case 0:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_DOT(ch[i])) {
				*(label_or_inst_p++) = ch[i];
				state = 1;
			}
			else if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
				row->type = RT_NULL;
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 1:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_NUM(ch[i])) {
				*(label_or_inst_p++) = ch[i];
				state = 2;
			}
			else if (IS_COLON(ch[i])) {
				*label_or_inst_p = 0;
				row->type = RT_LABEL;
				strcpy(row->u.label.name, label_or_inst_buff);
				state = 30;
			}
			else if (IS_SPACE(ch[i])) {
				*label_or_inst_p = 0;
                row->type = RT_INSTRUCTION;
                row->u.inst.argnum = argnum;
                strcpy(row->u.inst.name, label_or_inst_buff);
				state = 3;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*label_or_inst_p = 0;
                row->type = RT_INSTRUCTION;
                row->u.inst.argnum = argnum;
                strcpy(row->u.inst.name, label_or_inst_buff);
                return 1;
            }
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 2:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_NUM(ch[i])) {
				*(label_or_inst_p++) = ch[i];
			}
			else if (IS_SPACE(ch[i])) {
				*label_or_inst_p = 0;
				row->type = RT_INSTRUCTION;
				row->u.inst.argnum = argnum;
				strcpy(row->u.inst.name, label_or_inst_buff);
				state = 3;
			}
			else if (IS_COLON(ch[i])) {
				*label_or_inst_p = 0;
				row->type = RT_LABEL;
				strcpy(row->u.label.name, label_or_inst_buff);
				state = 30;
			}
			else if (IS_TERMINATOR(ch[i])) {
				*label_or_inst_p = 0;
                row->type = RT_INSTRUCTION;
                row->u.inst.argnum = argnum;
                strcpy(row->u.inst.name, label_or_inst_buff);
                return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		case 3:
			if (IS_COLON(ch[i])) {
                *label_or_inst_p = 0;
				row->type = RT_LABEL;
				strcpy(row->u.label.name, label_or_inst_buff);
				state = 30;
			}
			else if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
                *label_or_inst_p = 0;
                row->type = RT_INSTRUCTION;
                row->u.inst.argnum = argnum;
                strcpy(row->u.inst.name, label_or_inst_buff);
                return 1;
			}
			else {
				*(arg_p++) = ch[i];
				state = 11;
			}
			break;

        case 10:
            if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_NUM(ch[i])) {
                *(inst_p++) = ch[i];
            }
            else if (IS_SPACE(ch[i])) {
                *inst_p = 0;
                state = 11;
            }
            else if (IS_TERMINATOR(ch[i])) {
                *inst_p = 0;
                row->type |= RT_INSTRUCTION;
				row->u.label_inst.inst.argnum = argnum;
                strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name, row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                return 1;
            }
            else {
                sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
                PRINT_ERROR(current_error);
                return 0;
            }
		    break;

        case 11:
            if (IS_TERMINATOR(ch[i])) {
                if (argnum == 0) {
                    *arg_p = 0;
                    row->type |= RT_INSTRUCTION;
                    if (row->type == RT_INSTRUCTION)
                        row->u.inst.argnum = argnum + 1;
                    else row->u.label_inst.inst.argnum = argnum + 1;
                    strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name,
                           row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                    return 1;
                } else {
                    *arg_p = 0;
                    row->type |= RT_INSTRUCTION;
                    if (row->type == RT_INSTRUCTION)
                        row->u.inst.argnum = argnum + 1;
                    else row->u.label_inst.inst.argnum = argnum + 1;
                    strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name,
                           row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                    if (parse_arg(arg_buff, row->type == RT_INSTRUCTION ? &(row->u.inst.arg[argnum])
                                                                        : &(row->u.label_inst.inst.arg[argnum])) == 0)
                        return 0;
                    return 1;
                }
            }
            else if (IS_COMMA(ch[i])) {
                *arg_p = 0;
                row->type |= RT_INSTRUCTION;
                if (row->type == RT_INSTRUCTION) row->u.inst.argnum = argnum; else row->u.label_inst.inst.argnum = argnum;
                strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name, row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                if (parse_arg(arg_buff, row->type == RT_INSTRUCTION ? &(row->u.inst.arg[argnum]) : &(row->u.label_inst.inst.arg[argnum])) == 0)
                    return 0;
                argnum++;
                arg_p = arg_buff;
            }
            else {
                *(arg_p++) = ch[i];
                state = 12;
            }
            break;

        case 12:
            if (IS_COMMA(ch[i])) {
                *arg_p = 0;
                row->type |= RT_INSTRUCTION;
                if (row->type == RT_INSTRUCTION) row->u.inst.argnum = argnum; else row->u.label_inst.inst.argnum = argnum;
                strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name, row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                if (parse_arg(arg_buff, row->type == RT_INSTRUCTION ? &(row->u.inst.arg[argnum]) : &(row->u.label_inst.inst.arg[argnum])) == 0)
                    return 0;
                argnum++;
                arg_p = arg_buff;
                state = 11;
            }
            else if (IS_TERMINATOR(ch[i])) {
                *arg_p = 0;
                row->type |= RT_INSTRUCTION;
                if (row->type == RT_INSTRUCTION) row->u.inst.argnum = argnum + 1; else row->u.label_inst.inst.argnum = argnum + 1;
                strcpy(row->type == RT_INSTRUCTION ? row->u.inst.name : row->u.label_inst.inst.name, row->type == RT_INSTRUCTION ? label_or_inst_buff : inst_buff);
                if (parse_arg(arg_buff, row->type == RT_INSTRUCTION ? &(row->u.inst.arg[argnum]) : &(row->u.label_inst.inst.arg[argnum])) == 0)
                    return 0;
                return 1;
            }
            else {
                *(arg_p++) = ch[i];
            }
            break;

		case 30:
			if (IS_ALPHA(ch[i]) || IS_UNDERLINE(ch[i]) || IS_DOT(ch[i])) {
				*(inst_p++) = ch[i];
				state = 10;
			}
			else if (IS_SPACE(ch[i])) {
			}
			else if (IS_TERMINATOR(ch[i])) {
				row->type = RT_LABEL;
				strcpy(row->u.label.name, label_or_inst_buff);
				return 1;
			}
			else {
				sprintf(current_error, "syntax error near character \'%c\'", ch[i]);
				PRINT_ERROR(current_error);
				return 0;
			}
			break;

		}
	}
}

void print_row_info(Row_info * row) {
	int i;
	switch (row->type) {
	case RT_INSTRUCTION:
		printf("row->type = RT_INSTRUCTION\nrow->u.inst.name = %s\nrow->u.inst.argnum = %d\n", row->u.inst.name, row->u.inst.argnum);
		for (i = 0; i < row->u.inst.argnum; i++)
			print_arg_info(&(row->u.inst.arg[i]));
		break;

	case RT_LABEL:
		printf("row->type = RT_LABEL\nrow->u.label.name = %s\n", row->u.label.name);
		break;

	case RT_LABEL_AND_INSTRUCTION:
		printf("row->type = RT_LABEL_AND_INSTRUCTION\nrow->u.label_inst.label.name = %s\n", row->u.label_inst.label.name);
		printf("row->u.label_inst.inst.name = %s\nrow->u.label_inst.inst.argnum = %d\n", row->u.label_inst.inst.name, row->u.label_inst.inst.argnum);
		for (i = 0; i < row->u.label_inst.inst.argnum; i++)
			print_arg_info(&(row->u.label_inst.inst.arg[i]));
		break;

	case RT_NULL:
		printf("row->type = RT_NULL\n");
		break;

	default:
		printf("row->type = ILLEGAL\n");
		break;
	}
	putchar('\n');
}

void print_arg_info(Arg_info * arg) {
	switch (arg->type) {
	case AT_NULL:
		printf("arg->type = AT_NULL\n");
		break;

	case AT_REGISTER:
		printf("arg->type = AT_REGISTER, arg->u.reg = %d\n", arg->u.reg);
		break;

	case AT_NUM:
		printf("arg->type = AT_NUM, arg->u.num = %d\n", arg->u.num);
		break;

	case AT_LABEL_NAME:
		printf("arg->type = AT_LABEL_NAME, arg->u.label_name = %s\n", arg->u.label_name);
		break;

    case AT_STRING:
        printf("arg->type = AT_STRING, arg->u.string = %s\n", arg->u.string);
        break;

	default:
		printf("arg->type = ILLEGAL\n");
		break;
	}
}




