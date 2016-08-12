#ifndef WM_H
#define WM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#define HASHTABLESIZE (256*256)
#define MAXLEN 256
#define MS_SMALLEST ps->msSmallest

typedef struct wm_pattern_struct
{
	struct wm_pattern_struct *next;//指向下一个模式串
	unsigned char *psPat; //pattern array//模式串数组
	unsigned psLen; //length of pattern in bytes//模式串的长度
}WM_PATTERN_STRUCT;

#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)

typedef struct wm_struct//模式串集的结构
{
	WM_PATTERN_STRUCT *plist;//最开始存放模式串的地方
	WM_PATTERN_STRUCT *msPatArray; 
	unsigned short *msNumArray; 
	                                                  
	int msNumPatterns; //模式串的个数
	unsigned msNumHashEntries;
	HASH_TYPE *msHash;
	unsigned char* msShift; //
	HASH_TYPE *msPrefix; 
	int msSmallest; 
}WM_STRUCT;

//函数声明
WM_STRUCT  *wmNew();                                    
void wmFree(WM_STRUCT *ps);                             
int wmAddPattern(WM_STRUCT *ps,unsigned char *P,int m);   
int wmPrepPatterns(WM_STRUCT *ps);                        
void wmSearch(WM_STRUCT *ps,unsigned char *Tx,int n,int *nfound);  
 int  sortcmp( const void * e1, const void * e2 );
 int get_ms_smallest();
#endif
//DD1120628082773354
