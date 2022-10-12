/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include "synchconsole.h"
#include <stdint.h>
#include <stdlib.h>

#define LF ((char)10)
#define CR ((char)13)
#define TAB ((char)9)
#define SPACE ((char)' ')
#define MAX_NUM_LENGTH 11

char _numberBuffer[MAX_NUM_LENGTH + 2];


char isBlank(char c) { return c == LF || c == CR || c == TAB || c == SPACE; }

/**
 * Read and store characters in the _numberBuffer until blank
 * or end of file
 *
 * It will read at most MAX_NUM_LENGTH + 1 character
 **/
void readUntilBlank() {
    memset(_numberBuffer, 0, sizeof(_numberBuffer));
    char c = kernel->synchConsoleIn->GetChar();

    if (c == EOF) {//EOF end of file?
        DEBUG(dbgSys, "Unexpected end of file - number expected");
        return;
    }

    if (isBlank(c)) {
        DEBUG(dbgSys, "Unexpected white-space - number expected");
        return;
    }

    int n = 0;

    while (!(isBlank(c) || c == EOF)) {
        _numberBuffer[n++] = c;
        if (n > MAX_NUM_LENGTH) {
            DEBUG(dbgSys, "Number is too long");
            return;
        }
        c = kernel->synchConsoleIn->GetChar();
    }
}

/**
 * Return true of the interger equals to the
 * interger stored in the string
 **/
bool compareNumAndString(int integer, const char *s) {
    if (integer == 0) return strcmp(s, "0") == 0;

    int len = strlen(s);

    if (integer < 0 && s[0] != '-') return false;

    if (integer < 0) s++, --len, integer = -integer;

    while (integer > 0) {
        int digit = integer % 10;

        if (s[len - 1] - '0' != digit) return false;

        --len;
        integer /= 10;
    }

    return len == 0;
}



void SysHalt()
{
  kernel->interrupt->Halt();
}


int SysAdd(int op1, int op2)
{
  return op1 + op2;
}


int SysReadNum() {
    readUntilBlank();

    int len = strlen(_numberBuffer);
    // Read nothing -> return 0
    if (len == 0) return 0;

    // Check comment below to understand this line of code
    if (strcmp(_numberBuffer, "-2147483648") == 0) return INT32_MIN;

    bool nega = (_numberBuffer[0] == '-');//kiểm tra ký tự đầu tiên có phải là dấu âm hay ko
    int zeros = 0;
    bool is_leading = true;
    int num = 0;
    for (int i = nega; i < len; ++i) {
        char c = _numberBuffer[i];
        if (c == '0' && is_leading)//xet so 0 dung dau !!
            ++zeros;
        else
            is_leading = false;
        if (c < '0' || c > '9') {
            DEBUG(dbgSys, "Expected number but " << _numberBuffer << " found");
            return 0;
        }
        num = num * 10 + (c - '0');//c la 1 ky tu '0' cung la 1 ky tu tuong ung 30 (int)
    }

    // 00            01 or -0
    if (zeros > 1 || (zeros && (num || nega))) {
        DEBUG(dbgSys, "Expected number but " << _numberBuffer << " found");
        return 0;
    }

    if (nega)
        /**
         * This is why we need to handle -2147483648 individually:
         * 2147483648 is larger than the range of int32
         */
        num = -num;

    // It's safe to return directly if the number is small
    if (len <= MAX_NUM_LENGTH - 2) return num;

    /**
     * We need to make sure that number is equal to the number in the buffer.
     *
     * Ask: Why do we need that?
     * Answer: Because it's impossible to tell whether the number is bigger
     * than INT32_MAX or smaller than INT32_MIN if it has the same length.
     *
     * For example: 3 000 000 000.
     *
     * In that case, that number will cause an overflow. However, C++
     * doens't raise interger overflow, so we need to make sure that the input
     * string and the output number is equal.
     *
     */
    if (compareNumAndString(num, _numberBuffer))
        return num;
    else
        DEBUG(dbgSys,
              "Expected int32 number but " << _numberBuffer << " found");

    return 0;
}

void PrintNum(int number)
{
    bool negative=false;
    int size_num=0;
    int character=number;
    if (number<0) 
    {
        negative=true;
        character=-character;
    }
    for(int i=character;i!=0;i=i/10)
    size_num++;

    char *c=(char*) malloc(size_num);
    for(int i=size_num-1;i>=0;i--)
    {
        c[i]=character%10 +'0';//vi character day la dang int sang ascii. Co the thay '0' thanh 48
        character=character/10;
    }
    if (negative) kernel->synchConsoleOut->PutChar('-');


    for(int i=0;i<size_num;i++)
    {
        kernel->synchConsoleOut->PutChar(c[i]);
    }
    free(c);
    if (number==0) kernel->synchConsoleOut->PutChar('0');

    // kernel->synchConsoleOut->PutChar(13);
    // kernel->synchConsoleOut->PutChar(10);


}

int RandomNum()
{
    srand((unsigned int)time(NULL));
    int x;
    x=rand();
    return x;
}

void ascii()
{
    for (int i=0;i<=126;i++)
    {
        PrintNum(i);
        kernel->synchConsoleOut->PutChar(':');
        kernel->synchConsoleOut->PutChar(i);
        kernel->synchConsoleOut->PutChar(13);
        kernel->synchConsoleOut->PutChar(10);
    }
}

#endif /* ! __USERPROG_KSYSCALL_H__ */
