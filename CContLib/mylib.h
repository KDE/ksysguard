/*
    C Container Library
   
	Copyright (c) 1992-1999 Chris Schlaeger <cs@kde.org>
    
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA. 
*/

#if !defined __MYLIB__
#define __MYLIB__

typedef long INDEX;

typedef void (*DESTR_FUNC)(void*);
typedef int  (*COMPARE_FUNC)(void*, void*);
typedef void (*REPORT_FUNC)(INDEX, INDEX);
typedef int  (*CHECK_FUNC)(void*);

typedef enum
{
	CT_DCL = 0,
	CT_SCL = 1,
	CT_PGA = 2,
} CONTAINER_TYPE;

typedef struct
{
	void* (*new)(void);
	void  (*destruct)(void*, DESTR_FUNC);
	int   (*check)(void*, CHECK_FUNC);
	int   (*insert)(void*, void*, INDEX);
	void* (*get)(void*, INDEX);
	void* (*remove)(void*, INDEX);
	INDEX (*level)(void*);
	int   (*swop)(void*, INDEX, INDEX);
} CONT_TOOLS;

typedef struct
{
	CONT_TOOLS* tools;
	void*       handle;
} CONT;
typedef CONT* CONTAINER;

/*
 *  Module CONTAINER
 */
 
void      bsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report);
int       check_ctnr(CONTAINER ctnr, CHECK_FUNC check);
void      destr_ctnr(CONTAINER ctnr, DESTR_FUNC destr_obj);
void*     get_ctnr(CONTAINER ctnr, INDEX pos);
int       insert_ctnr(CONTAINER ctnr, void* object, INDEX pos);
INDEX     level_ctnr(CONTAINER ctnr);
CONTAINER new_ctnr(CONTAINER_TYPE type);
void*     plop_ctnr(CONTAINER ctnr);
void*     pop_ctnr(CONTAINER ctnr);
int       push_ctnr(CONTAINER ctnr, void* object);
void      qsort_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, REPORT_FUNC report);
void*     remove_ctnr(CONTAINER ctnr, INDEX pos);
INDEX     search_ctnr(CONTAINER ctnr, COMPARE_FUNC compare, void* pattern);

/*
 *  Module FIO
 */

#if !defined(__STDIO)
#include <stdio.h>
#endif

size_t fcread(void *ptr, size_t elem_size, size_t count, FILE* istrm);
size_t fcwrite(void *ptr, size_t elem_size, size_t count, FILE* istrm);
void   hidefile(char* fname);
size_t q_filesize(const char filename[]);
int    rdc_byte(char* byte, FILE* istrm);
int    rdc_long(long* lng, FILE* istrm);
int    rdc_str(char* str, size_t len, FILE* istrm);
int    rdc_word(int* word, FILE* istrm);
int    rd_byte(char* byte, FILE* istrm);
int    rd_long(long* lng, FILE* istrm);
int    rd_str(char* str, size_t len, FILE* istrm);
int    rd_word(int* word, FILE* istrm);
int    wrc_byte(char byte, FILE* ostrm);
int    wrc_long(long lng, FILE* ostrm);
int    wrc_word(int word, FILE* ostrm);
int    wr_byte(char byte, FILE* ostrm);
int    wr_long(long lng, FILE* ostrm);
int    wr_word(int word, FILE* ostrm);

extern unsigned int Codepat;

/*
 *  Module LOWLEVEL
 */

#define BAD_CALL 1 /* Debug level 1 reports failed function calls */
 
#define NIL     ((void*) 0L)
#define PRN_1	0	/* BIOS Device Drucker 1 */
#define AUX     1	/* variables BIOS Device */
#define MODEM1  6
#define MODEM2  7
#define SERIAL2 8
#define SCC_A	9	/* BIOS Device SCC Channel A */

typedef struct
{
	int baudrate;
	int parity;
	int stopbits;
	int databits;
	int handshake;
} AUX_PARM_BLOCK;

void   _lowlevel(void);

int    bconin(int dev);
int    bconstat(int dev);
void   bconout(int dev, int c);
long   bcostat(int dev);
void   delete(void *allocation);
void*  new(size_t memory);
void   pause(int millisec);
void   rprt_err(int level, const char* text);
int    rs232conf(int port, AUX_PARM_BLOCK* apb);

/*
 * Modul TIMESTUF
 */

long         get_time(void);
void         strfgtime(char *str, char* format, long timemark);
void         time_str(char *str, char *format);
int          weekday(long timeshift);
unsigned int tos_date(long time);
unsigned int tos_time(long time);
unsigned int dmy2tos(int day, int mon, int year);
void         tos2dmy(int tos, int* day, int* mon, int* year);
unsigned int smh2tos(int sec, int min, int hour);
void         tos2smh(int tos, int* sec, int* min, int* hour);
unsigned int sub_time(unsigned int time1, unsigned int time2);

/*
 * Modul System
 */

char sbpeek(long addr);
void sbpoke(long addr, char value);
long slpeek(long addr);
void slpoke(long addr, long value);
int  speek(long addr);
void spoke(long addr, int value);

#endif
