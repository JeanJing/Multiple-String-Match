#ifndef WM_H
#define WM_H
#define HASHTABLESIZE (256*256)
#define MAXLEN 256
#define MS_SMALLEST ps->msSmallest
#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)
int nline=1;
#define MAXN 10001 //模式串的最大长度MAXN - 1
#define MAXM 51    //单词最大长度为MAXM - 1
 //static  int table_size = 70000; // 这里设定数组的大小
	// 实验结果表明差别不是很大
int  seed = 1233;// 它給的范围很大，这个再确定
 int shift_L = 7;
 int shift_R = 2;
#define HASHTABLE_THRESHOLD 20






typedef struct wm_pattern_struct
{
	struct wm_pattern_struct *next;//指向下一个模式串
	unsigned char *psPat; //pattern array//模式串数组
	unsigned psLen; //length of pattern in bytes//模式串的长度
}WM_PATTERN_STRUCT;

#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)



typedef struct subHashEntry{
	int location;
	struct subHashEntry *next;
}SUB_HASH_ENTRY;
typedef struct hash_struct{
	bool  isHashTable;
	union hashEntry{//这个类型可以改成HASH_TYPE
		int location;
	    SUB_HASH_ENTRY **subhashTable;
	} HASH_ENTRY;
}HASH_STRUCT;





typedef struct wm_struct//模式串集的结构
{
	WM_PATTERN_STRUCT *plist;//最开始存放模式串的地方
	WM_PATTERN_STRUCT *msPatArray; 
	unsigned short *msNumArray; 
	                                                  
	int msNumPatterns; //模式串的个数
	unsigned msNumHashEntries;
	HASH_TYPE *msHash;
	//新的hash表，在之前的基础上构建
	HASH_STRUCT **newHash;
	unsigned char* msShift; //
	HASH_TYPE *msPrefix; 
	int msSmallest; 
}WM_STRUCT;






#endif