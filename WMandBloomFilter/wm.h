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

#define HASHTABLE_THRESHOLD_NO1 20
#define HASHTABLE_THRESHOLD_NO2 25000//д������ͻȻ��������㷨���ܲ�����ô�ĸ�Ч���
#define m_n_ratio 4
 //bloom_filter��һЩ����
typedef unsigned char uint8_t;
#define HASH_NUM 3
#define offset_basis 2166136261
inline unsigned int fnv_32a_buf(void *buf, int len);
inline unsigned int murMurHash(void *key, int len);
typedef unsigned int (*hash_function)(void *buf, int len);
hash_function hash_F[2] = {fnv_32a_buf, murMurHash};



typedef struct wm_pattern_struct
{
	struct wm_pattern_struct *next;//ָ����һ��ģʽ��
	unsigned char *psPat; //pattern array//ģʽ������
	unsigned psLen; //length of pattern in bytes//ģʽ���ĳ���
}WM_PATTERN_STRUCT;

#define HASH_TYPE int
#define SHIFTTABLESIZE (256*256)


//��hash��Ľṹ
typedef struct subHashEntry{
	int location;
	struct subHashEntry *next;
}SUB_HASH_ENTRY;

//�Ӳ�¡�������Ľṹ
typedef struct bloom_filter {
    uint8_t *bits;
    int size;
	SUB_HASH_ENTRY **bitEntry;//��ʼ��Ϊһ������Ϊsize*8��һ��Ԫ������ΪSUB_HASH_ENTRY������
}BLOOM_FILTER;
typedef  bloom_filter * bloom_t;


typedef struct hash_struct{
	char flag;//��־ȡֵΪ1,2,3��ע�ⲻ�Ǵ�0��ʼ��
	union hashEntry{//������Ϳ��Ըĳ�HASH_TYPE
		int location;
	    SUB_HASH_ENTRY **subhashTable;
		bloom_t bloom_filter;
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