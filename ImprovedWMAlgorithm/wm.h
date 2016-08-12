#ifndef WM_H
#define WM_H
#define HASHTABLESIZE (256*256)
#define MAXLEN 256
#define MS_SMALLEST ps->msSmallest
#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)
int nline=1;
#define MAXN 10001 //ģʽ������󳤶�MAXN - 1
#define MAXM 51    //������󳤶�ΪMAXM - 1
 //static  int table_size = 70000; // �����趨����Ĵ�С
	// ʵ������������Ǻܴ�
int  seed = 1233;// ���o�ķ�Χ�ܴ������ȷ��
 int shift_L = 7;
 int shift_R = 2;
#define HASHTABLE_THRESHOLD 20






typedef struct wm_pattern_struct
{
	struct wm_pattern_struct *next;//ָ����һ��ģʽ��
	unsigned char *psPat; //pattern array//ģʽ������
	unsigned psLen; //length of pattern in bytes//ģʽ���ĳ���
}WM_PATTERN_STRUCT;

#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)



typedef struct subHashEntry{
	int location;
	struct subHashEntry *next;
}SUB_HASH_ENTRY;
typedef struct hash_struct{
	bool  isHashTable;
	union hashEntry{//������Ϳ��Ըĳ�HASH_TYPE
		int location;
	    SUB_HASH_ENTRY **subhashTable;
	} HASH_ENTRY;
}HASH_STRUCT;





typedef struct wm_struct//ģʽ�����Ľṹ
{
	WM_PATTERN_STRUCT *plist;//�ʼ���ģʽ���ĵط�
	WM_PATTERN_STRUCT *msPatArray; 
	unsigned short *msNumArray; 
	                                                  
	int msNumPatterns; //ģʽ���ĸ���
	unsigned msNumHashEntries;
	HASH_TYPE *msHash;
	//�µ�hash����֮ǰ�Ļ����Ϲ���
	HASH_STRUCT **newHash;
	unsigned char* msShift; //
	HASH_TYPE *msPrefix; 
	int msSmallest; 
}WM_STRUCT;






#endif