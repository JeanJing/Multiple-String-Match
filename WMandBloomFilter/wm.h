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

#define HASHTABLE_THRESHOLD_NO1 20
#define HASHTABLE_THRESHOLD_NO2 25000//写到这里突然觉得这个算法可能不是那么的高效真的
#define m_n_ratio 4
 //bloom_filter的一些参数
typedef unsigned char uint8_t;
#define HASH_NUM 3
#define offset_basis 2166136261
inline unsigned int fnv_32a_buf(void *buf, int len);
inline unsigned int murMurHash(void *key, int len);
typedef unsigned int (*hash_function)(void *buf, int len);
hash_function hash_F[2] = {fnv_32a_buf, murMurHash};



typedef struct wm_pattern_struct
{
	struct wm_pattern_struct *next;//指向下一个模式串
	unsigned char *psPat; //pattern array//模式串数组
	unsigned psLen; //length of pattern in bytes//模式串的长度
}WM_PATTERN_STRUCT;

#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)


//子hash表的结构
typedef struct subHashEntry{
	int location;
	struct subHashEntry *next;
}SUB_HASH_ENTRY;

//子布隆过滤器的结构
typedef struct bloom_filter {
    uint8_t *bits;
    int size;
	SUB_HASH_ENTRY **bitEntry;//初始化为一个长度为size*8的一个元素类型为SUB_HASH_ENTRY的数组
}BLOOM_FILTER;
typedef  bloom_filter * bloom_t;


typedef struct hash_struct{
	char flag;//标志取值为1,2,3（注意不是从0开始）
	union hashEntry{//这个类型可以改成HASH_TYPE
		int location;
	    SUB_HASH_ENTRY **subhashTable;
		bloom_t bloom_filter;
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