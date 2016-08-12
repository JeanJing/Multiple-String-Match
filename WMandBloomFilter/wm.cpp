#include "wm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <stdarg.h>
#include <iostream>
using  namespace std;
#define MAXPATLEN 200
static WM_STRUCT *p;  

static unsigned HASH16(unsigned char *T)
{
	/*/
	printf("T:%c\n",*(T));
	getchar();
	printf("T+1:%c\n",*(T+1));
	getchar();
	printf("T<<8:%c\n",(int)((*T)<<8));
	getchar();
	printf("HASH16:%d\n",((*T)<<8) | *(T+1));
	getchar();
	//*/
	return (unsigned short) (((*T)<<8) | *(T+1)); //对第一个字符左移8位，然后与第二个字符异或运算
}
//str是要hash的字符串，str_size是字符串的长度，table_size是表长
static int wiseHash(unsigned char* str, int str_size, int table_size) {
	int cur_h = seed;
	for (int i = 0; i < str_size; i++) {
		int temp = 0;
		temp = (cur_h << shift_L) + (cur_h >> shift_R) + str[i];
		cur_h = cur_h ^ (temp);
	}
	if(cur_h< 0)
		cur_h = - cur_h;
	cur_h = cur_h % table_size;
	return cur_h;
}





WM_STRUCT * wmNew()
{
	WM_STRUCT *p=(WM_STRUCT *)malloc(sizeof(WM_STRUCT));
	if(!p) return 0;
	p->msNumPatterns=0;   //模式串的个数,初始为0
	p->msSmallest=1000;   //最短模式串的长度
	return p;
}
int wmAddPattern(WM_STRUCT *ps,unsigned char *q,int m)
{
	WM_PATTERN_STRUCT *p;  //定义一个子串结构
	p=(WM_PATTERN_STRUCT *)malloc(sizeof(WM_PATTERN_STRUCT));
	if(!p) return -1;

	p->psPat=(unsigned char*)malloc(m+1); //据子串数组的长度分配空间
	memset(p->psPat+m,0,1);	//最后一个位置设置为结束字符“/0” 
	memcpy(p->psPat,q,m); //拷贝q到子串结构数组中
	p->psLen=m;           //子串长度赋值
	ps->msNumPatterns++; //模式串集的子串个数增1
	if(p->psLen < (unsigned)ps->msSmallest) ps->msSmallest = p->psLen; //重新确定最短字符串长度

	p->next=ps->plist; //将新增子串加入字符串集列表中。队列形式，新增在队列头部
	ps->plist=p;

	return 0;
}
void read_pat_to_wm( WM_STRUCT *p, const char *filename){
	char pat[MAXPATLEN + 1];
	int pat_len = 0;
	char *line_break;
	FILE *fp_pats = fopen(filename, "r");
	while (fgets(pat, sizeof(pat), fp_pats) != NULL ) {
		if ((line_break = strchr(pat, '\n')) != NULL)
			*line_break = '\0';
		wmAddPattern(p,(unsigned char *)pat, strlen(pat) );
		pat_len++;
	}
}
int get_ms_smallest(){
	return p->msSmallest;
}
//这个排序是按照前最短字符串的后2个字符的hash值排序
int  sortcmp( const void * e1, const void * e2 )
{
	WM_PATTERN_STRUCT *r1= (WM_PATTERN_STRUCT*)e1;
	WM_PATTERN_STRUCT *r2= (WM_PATTERN_STRUCT*)e2;
	int ms_smallest = get_ms_smallest();
	return (HASH16(&(r1->psPat[ms_smallest-2])) - HASH16(&(r2->psPat[ms_smallest-2])));
}

static void wmPrepShiftTable(WM_STRUCT *ps)
{
	int i;
	unsigned short m,k,cindex;
	unsigned shift;
	m=(unsigned short)ps->msSmallest;
	ps->msShift=(unsigned char*)malloc(SHIFTTABLESIZE*sizeof(char));
	if(!ps->msShift)
		return;

	for(i=0;i<SHIFTTABLESIZE;i++)	//初始化Shift表，初始值为最短字符串的长度
	{
		ps->msShift[i]=(unsigned)(m-2+1);
	}

	for(i=0;i<ps->msNumPatterns;i++)	//针对每个子串预处理
	{
		for(k=0;k<m-1;k++)
		{
			shift=(unsigned short)(m-2-k);
			cindex=((ps->msPatArray[i].psPat[k]<<8) | (ps->msPatArray[i].psPat[k+1]));//B为2
			if(shift < ps->msShift[cindex])
				ps->msShift[cindex] = shift;//k=m-2时，shift=0，
		}
	}
}
static void wmPrepPrefixTable(WM_STRUCT *ps)//建立Prefix表
{
	int i;
	ps->msPrefix=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)* ps->msNumPatterns);	//分配空间长度为所有子串的个数*
	if(!ps->msPrefix)
	{
		printf("No memory in wmPrepPrefixTable()\n");
		return;
	}

	for(i=0;i<ps->msNumPatterns;i++)	//哈希建立Prefix表
	{
		ps->msPrefix[i]=HASH16(ps->msPatArray[i].psPat);//对每个模式串的前缀进行哈希
	}
} 
//创建一个bloom过滤器，为基本的变量赋值，现在规定bloom构造函数的这个参数size，只是表示hash到这个bloom的模式的个数
bloom_t bloom_create(int size) {
	bloom_t res = (bloom_t)calloc(1, sizeof(BLOOM_FILTER));
	int bits_size = ceil((double)(size * m_n_ratio/8));
	res->size =  bits_size* 8;
	res->bits = (uint8_t *)malloc(bits_size*sizeof(uint8_t));
	for(int i = 0; i< bits_size; i++)
		res->bits[i] = 0;
	res->bitEntry = (SUB_HASH_ENTRY**)malloc(sizeof(SUB_HASH_ENTRY*) *res->size);
	for(int i = 0; i <res->size; i++)
		res->bitEntry[i] =NULL;
	return res;
}

static inline unsigned int fnv_32a_buf(void *buf, int len)  
{  
	unsigned int hval = offset_basis;
	unsigned char *bp = (unsigned char *) buf;  
	unsigned char *be = bp + len;  
	while (bp < be) {  
		hval ^= (int) *bp++;  
		hval += (hval << 1) + (hval << 4) + (hval << 5) +  
			(hval << 7) + (hval << 8) + (hval << 40);  
	}

	return hval;  
}
inline unsigned int murMurHash(void *key, int len)
{
	const unsigned int m = 0x5bd1e995;
	const int r = 24;
	const int seed = 97;
	unsigned int h = seed ^ len;
	// Mix 4 bytes at a time into the hash
	const unsigned char *data = (const unsigned char *)key;
	while(len >= 4)
	{
		unsigned int k = *(unsigned int *)data;
		k *= m; 
		k ^= k >> r; 
		k *= m; 
		h *= m; 
		h ^= k;
		data += 4;
		len -= 4;
	}
	// Handle the last few bytes of the input array
	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};
	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;
	return h;
}
void bloom_add(WM_STRUCT *p, bloom_t filter,  int number) {
	unsigned int hash_value[HASH_NUM];
	uint8_t *bits = filter->bits;
	SUB_HASH_ENTRY** entryArray = filter->bitEntry;
	SUB_HASH_ENTRY* entryList;

	void * item = p->msPatArray[number].psPat;
	int str_len = p->msSmallest;
	int i = 0;
	for(int i =0; i < 2; i++){
		hash_value[i] = hash_F[i](item, str_len) % (filter->size) ;
		bits[hash_value[i] >> 3] |= 1 << (hash_value[i] & 7);
		//entryList = entryArray[hash_value[i]];
		if(entryArray[hash_value[i]] == NULL){
			entryList = (SUB_HASH_ENTRY*)malloc(sizeof(SUB_HASH_ENTRY));
			entryList->location = number;
			entryList ->next = NULL;
			entryArray[hash_value[i]] = entryList;
		}else{
			entryList = entryArray[hash_value[i]];
			while(entryList->next != NULL){
				entryList = entryList->next;
			}
			entryList->next = (SUB_HASH_ENTRY*)malloc(sizeof(SUB_HASH_ENTRY));

			entryList->next->location = number;
			entryList->next ->next = NULL;
		}
	}
	for(i = 2; i < HASH_NUM;i++){
		hash_value[i] = (hash_value[i - 2] + hash_value[i - 1] * (i + 1))%(filter->size);
		bits[hash_value[i] >> 3] |= 1 << (hash_value[i] & 7);
		
		if(entryArray[hash_value[i]] == NULL){
			entryList = (SUB_HASH_ENTRY*)malloc(sizeof(SUB_HASH_ENTRY));
			entryList->location = number;
			entryList ->next = NULL;
			entryArray[hash_value[i]] = entryList;
		}else{
			entryList = entryArray[hash_value[i]];
			while(entryList->next != NULL){
				entryList = entryList->next;
			}
			entryList->next = (SUB_HASH_ENTRY*)malloc(sizeof(SUB_HASH_ENTRY));
			entryList->next->location = number;
			entryList->next ->next = NULL;
		}
	}
}
static inline bool bloom_match(WM_STRUCT *ps, bloom_t filter,  void * str, int str_len, int * nfound) {
	uint8_t *bits = filter->bits;
	unsigned int hash_value[HASH_NUM];
	int text_prefix;
	SUB_HASH_ENTRY* entryList;
	unsigned char *px,*qx;


	int i = 0;
	for(int i =0; i < 2; i++){
		hash_value[i] = hash_F[i](str, str_len) % (filter->size) ;
		entryList = filter->bitEntry[hash_value[i]];
		text_prefix=HASH16((unsigned char *)str);
		int found  = 0;
		while(entryList != NULL){
			if(ps->msPrefix[entryList->location] != text_prefix){
				entryList = entryList->next;
				continue;
			}else{//如果前缀相同就比较后缀
				px=(ps->msPatArray[entryList->location]).psPat;	//取patrn的字串
				qx=(unsigned char *)str;
				while(*(px++)==*(qx++) && *(qx-1)!='\0');	//整个模式串进行比较,，因为有\0的控制所以
				if(*(px-1)=='\0')	//匹配到了结束位置，说明匹配成功
				{
					//printf("%s\n", ps->msPatArray[entryList->location].psPat);
				   //printf("Match pattern \"%s\" at line %d column %d\n",(ps->msPatArray[hash_list->location]).psPat,nline,T-Tx+1);
					//nfound++;
					found++;
				}
				entryList = entryList->next;

			}
		}
		if(found > 0 ){
			*nfound += found;
			return true;
		}

			
	}
	for(i = 2; i < HASH_NUM;i++){
		hash_value[i] = (hash_value[i - 2] + hash_value[i - 1] * (i + 1))% (filter->size);
		entryList = filter->bitEntry[hash_value[i]];
		text_prefix=HASH16((unsigned char *)str);
		int found  = 0;
		while(entryList != NULL){
			if(ps->msPrefix[entryList->location] != text_prefix){
				entryList = entryList->next;
				continue;
			}else{//如果前缀相同就比较后缀
				px=(ps->msPatArray[entryList->location]).psPat;	//取patrn的字串
				qx=(unsigned char *)str;
				while(*(px++)==*(qx++) && *(qx-1)!='\0');	//整个模式串进行比较,，因为有\0的控制所以
				if(*(px-1)=='\0')	//匹配到了结束位置，说明匹配成功
				{
					//printf("%s\n",px);
				   //printf("Match pattern \"%s\" at line %d column %d\n",(ps->msPatArray[hash_list->location]).psPat,nline,T-Tx+1);
					//nfound++;

					found++;
				}
				entryList = entryList->next;

			}
		}
		if(found > 0 ){
			*nfound += found;
			return true;
		}
	}
	return false;
}
bool bloom_test( bloom_t filter,  void * str, int str_len) {
	uint8_t *bits = filter->bits;
	unsigned int hash_value[HASH_NUM];

	int i = 0;
	for(int i =0; i < 2; i++){
		hash_value[i] = hash_F[i](str, str_len) % (filter->size) ;
		if(!(filter->bits[hash_value[i] >> 3]  & 1<<( hash_value[i] &7)))
			return false;
	}
	for(i = 2; i < HASH_NUM;i++){
		hash_value[i] = (hash_value[i - 2] + hash_value[i - 1] * (i + 1))% (filter->size );
		if(!(filter->bits[hash_value[i] >> 3]  & 1<<( hash_value[i] &7)))
			return false;
	}
	return true;
}


static void wmPrepHashedPatternGroups(WM_STRUCT *ps)
{
	unsigned sindex,hindex,ningroup;
	int i;
	int m=ps->msSmallest;
	ps->msNumHashEntries=HASHTABLESIZE;	//HASH表的大小
	ps->msHash=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)* ps->msNumHashEntries);	//HASH表
	if(!ps->msHash)
	{
		printf("No memory in wmPrepHashedPatternGroups()\n");
		return;
	}

	for(i=0;i<(int)ps->msNumHashEntries;i++)	//HASH表预处理初始化，全部初始化为(HASH_TYPE)-1
	{
		ps->msHash[i]=(HASH_TYPE)-1;
	}
	//FILE * outfile = fopen("hashstatic.txt","w");
	for(i=0;i<ps->msNumPatterns;i++)	//针对所有子串进行HASH预处理
	{
		hindex=HASH16(&ps->msPatArray[i].psPat[m-2]);	//对模式子串的最后两个字符计算哈希值（匹配）
		sindex=ps->msHash[hindex]=i;
		ningroup=1;
		//此时哈希表已经有序了 
		while((++i<ps->msNumPatterns) && (hindex==HASH16(&ps->msPatArray[i].psPat[m-2])))	//找后缀相同的子串数
			ningroup++;
		ps->msNumArray[sindex]=ningroup;	//第i个子串，其后的子模式串与其后缀2字符相同子串的个数
		//fprintf(outfile, "%d%s%d%s%d\n",hindex, "  ",i,"  ",ningroup);
		i--;
	}
	//构建新的hash表
	ps->newHash=(HASH_STRUCT**)malloc(sizeof(HASH_STRUCT*)* ps->msNumHashEntries);	//HASH表
	if(!ps->newHash)
	{
		printf("No memory in wmPrepHashedPatternGroups()\n");
		return;
	}
	for(int n = 0; n < ps->msNumHashEntries;n++){
		if(ps->msHash[n]== -1)
			ps->newHash[n] = NULL;
		else{
			ps->newHash[n] = (HASH_STRUCT*)malloc(sizeof(HASH_STRUCT));
			//判断这个hash槽中元素个数
			int numberOfEntryInSlot = ps->msNumArray[ps->msHash[n]];
			if(numberOfEntryInSlot <= HASHTABLE_THRESHOLD_NO1){

				(ps->newHash[n])->flag = 1;

				ps->newHash[n]->HASH_ENTRY.location = ps->msHash[n];
			}else{			
				if(numberOfEntryInSlot > HASHTABLE_THRESHOLD_NO1 && numberOfEntryInSlot <= HASHTABLE_THRESHOLD_NO2){

					ps->newHash[n]->flag = 2;

					ps->newHash[n]->HASH_ENTRY.subhashTable = (SUB_HASH_ENTRY **)malloc(sizeof(SUB_HASH_ENTRY*) * numberOfEntryInSlot);
					//对子hash表要进行一个简单的赋值
					for(int j =0; j< numberOfEntryInSlot; j++)
						ps->newHash[n]->HASH_ENTRY.subhashTable[j] = NULL;
					//接下来将那个元素依次映射到新建的hash表了
					for(int j = ps->msHash[n]; j<ps->msHash[n] + numberOfEntryInSlot;j++){
						int slot_number = wiseHash(ps->msPatArray[j].psPat, ps->msSmallest,numberOfEntryInSlot);//这个hash函数我们可以
						//得到了hash槽的号，现在第一步判断该槽目前有没有元素
						if(ps->newHash[n]->HASH_ENTRY.subhashTable[slot_number]==NULL){
							SUB_HASH_ENTRY *sub_hash_entry =(SUB_HASH_ENTRY *)malloc(sizeof(SUB_HASH_ENTRY));
							sub_hash_entry->location = j;
							sub_hash_entry->next = NULL;
							ps->newHash[n]->HASH_ENTRY.subhashTable[slot_number]=sub_hash_entry;
						}else{
							SUB_HASH_ENTRY *current_hash_entry = ps->newHash[n]->HASH_ENTRY.subhashTable[slot_number];
							while(current_hash_entry->next!=  NULL){
								current_hash_entry = current_hash_entry->next;
							}
							current_hash_entry->next = (SUB_HASH_ENTRY *)malloc(sizeof(SUB_HASH_ENTRY));
							current_hash_entry->next->location = j;
							current_hash_entry->next->next = NULL;
						}
					}

				}else{
					//cout << "有bloom过滤器被构造"<< endl;
					ps->newHash[n]->flag = 3;
					ps->newHash[n]->HASH_ENTRY.bloom_filter = bloom_create(numberOfEntryInSlot);//这样的话m/n等于4，再乘以0.69，hash函数的取值就为3
					for(int j = ps->msHash[n]; j<ps->msHash[n] + numberOfEntryInSlot;j++){
						bloom_add(ps,ps->newHash[n]->HASH_ENTRY.bloom_filter,j);
					}
				}
			}

		}
	}


}

int wmPrepPatterns(WM_STRUCT *ps)
{
	int kk;
	WM_PATTERN_STRUCT *plist;
	ps->msPatArray=(WM_PATTERN_STRUCT*)calloc(sizeof(WM_PATTERN_STRUCT),ps->msNumPatterns);
	if(!ps->msPatArray)
		return -1;

	ps->msNumArray=(unsigned short*)calloc(sizeof(short),ps->msNumPatterns);//这里我还不太理解
	if(!ps->msNumArray)
		return -1;

	for(kk=0,plist=ps->plist;plist!=NULL && kk<ps->msNumPatterns;plist=plist->next)
	{
		memcpy(&ps->msPatArray[kk++],plist,sizeof(WM_PATTERN_STRUCT));
	}
	static int m = ps->msSmallest;
	//sort(ps);
	qsort(ps->msPatArray,ps->msNumPatterns,sizeof(WM_PATTERN_STRUCT),sortcmp);	//哈希排序


	wmPrepHashedPatternGroups(ps);	//哈希表  
	wmPrepShiftTable(ps);	//shift表
	wmPrepPrefixTable(ps);	//Prefix表
	return 0;
}
static int wmGroupMatch(WM_STRUCT *ps,
						HASH_STRUCT* lindex,//lindex为后缀哈希值相同的那些模式子串中的一个模式子串的index,那个改进之后这个值就是指向一个子hash表，这个子hash是一个链表数组
						unsigned char *Tx,
						unsigned char *T, int numberInHash)
{
	WM_PATTERN_STRUCT *patrn; 
	WM_PATTERN_STRUCT *patrnEnd;
	int text_prefix;
	unsigned char *px,*qx;
	int nfound = 0;
	if(lindex->flag == 1){
		int index_of_first_pattern = lindex->HASH_ENTRY.location;
		patrn=&ps->msPatArray[index_of_first_pattern];
		patrnEnd=patrn+ps->msNumArray[index_of_first_pattern];

		text_prefix=HASH16(T);


		for(;patrn<patrnEnd;patrn++)
		{
			if(ps->msPrefix[index_of_first_pattern++]!=text_prefix)
				continue;
			else	//如果后缀哈希值相同，则
			{
				px=patrn->psPat;	//取patrn的字串
				qx=T;
				while(*(px++)==*(qx++) && *(qx-1)!='\0');	//整个模式串进行比较
				if(*(px-1)=='\0')	//匹配到了结束位置，说明匹配成功
				{
					//printf("Match pattern \"%s\" at line %d column %d\n",patrn->psPat,nline,T-Tx+1);

					nfound++;
				}
			}
		}
	}else{
		if(lindex->flag == 2){
			int hash_value = wiseHash(T,ps->msSmallest,ps->msNumArray[ps->msHash[numberInHash]]);
		SUB_HASH_ENTRY* hash_list = lindex->HASH_ENTRY.subhashTable[hash_value];
		text_prefix=HASH16(T);
		while(hash_list!=NULL){
			if(ps->msPrefix[hash_list->location] != text_prefix){
				hash_list= hash_list->next;
				continue;
			}
			else{//如果前缀相同就开始比较后缀
				px=(ps->msPatArray[hash_list->location]).psPat;	//取patrn的字串
				qx=T;
				while(*(px++)==*(qx++) && *(qx-1)!='\0');	//整个模式串进行比较
				if(*(px-1)=='\0')	//匹配到了结束位置，说明匹配成功
				{
					//printf("Match pattern \"%s\" at line %d column %d\n",(ps->msPatArray[hash_list->location]).psPat,nline,T-Tx+1);

					nfound++;
				}
				hash_list= hash_list->next;
			}

		}
		}else{
			if(bloom_test( lindex->HASH_ENTRY.bloom_filter,T, ps->msSmallest))//当测试结果是1的时候需要进一步的检查
				bloom_match(ps,lindex->HASH_ENTRY.bloom_filter,T, ps->msSmallest,&nfound);
					//printf("Match pattern \"%s\" at line %d column %d\n",(ps->msPatArray[hash_list->location]).psPat,nline,T-Tx+1);
					//这个输出是有问题的，所以先按照个数检验代码的正确性
					
				
		}


	}
	return nfound;
}
void wmSearch(WM_STRUCT *ps,unsigned char *Tx,int n,int *nfound)
{
	*nfound = 0;
	int Tleft,tshift;
	HASH_STRUCT * lindex;
	unsigned char *T,*Tend,*window;
	int found_per = 0;
	Tleft=n;//这个应该是表示还剩下几个字符没有被比较过
	Tend=Tx+n;
	if(n < ps->msSmallest)	/*被查找的字符串序列比最小模式子串还短，
							显然是不可能查找到的，直接退出*/
							return;
	//window相当于窗口的末尾
	for(T=Tx,window=Tx+ps->msSmallest-1;window<Tend;T++,window++,Tleft--)
	{
		tshift = ps->msShift[(*(window-1)<<8) | *window];
		while(tshift)//当tshift!=0,无匹配，那就根据这个跳转值完成跳转，直到，找到跳转值为0的项
		{
			window+=tshift;
			T+=tshift;
			Tleft-=tshift;
			if(window>Tend) return;
			tshift=ps->msShift[(*(window-1)<<8) | *window];
		}
		//tshift=0，表明后缀哈希值已经相同

		int numberInHash = ((*(window-1)<<8) | *window);

		if((lindex=ps->newHash[numberInHash])==NULL) continue;
		lindex=ps->newHash[numberInHash];//如果哈希值存在
		found_per =wmGroupMatch(ps,lindex,Tx,T,numberInHash);//后缀哈希值相同，比较前缀及整个模式串
		(*nfound) = (*nfound) +found_per;
	}
}


void terminate(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(stderr,format, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}
char *read_text(char const *text_file, unsigned *text_len)
{
	FILE *fp_text;
	char *text;

	if ((fp_text = fopen(text_file, "rb")) == NULL)
		terminate("can not open text file!");

	fseek(fp_text, 0, SEEK_END);
	*text_len = ftell(fp_text);
	rewind(fp_text);

	if ((text = (char *)malloc((*text_len +1)*sizeof(char))) == NULL)
		terminate("无法为文本分 配内存!\n");
	if(fread(text, *text_len, 1, fp_text) != 1)
		fprintf(stderr, "Can not fread file: %s\n", text_file);;
	text[*text_len] = '\0';
	return text;
}




#define TEXT_FILE_NAME "D:/patterns/rand_text500MB" 
#define RAND_TEXT_FILE_NAME "D:/patterns/rand_text.200MB" 
#define PATS_FILE_PATH "D:/patterns/"

char* pats_lsp[] = {PATS_FILE_PATH"r10000000"};/* PATS_FILE_PATH"r2000000",PATS_FILE_PATH"r3000000"};/*,PATS_FILE_PATH"r4000000",
	PATS_FILE_PATH"r5000000",PATS_FILE_PATH"r6000000",PATS_FILE_PATH"r7000000"};
/*char* pats_lsp[] = {PATS_FILE_PATH"s10000", PATS_FILE_PATH"s20000",PATS_FILE_PATH"s30000",PATS_FILE_PATH"s40000",
PATS_FILE_PATH"s50000",PATS_FILE_PATH"s60000",PATS_FILE_PATH"s70000",PATS_FILE_PATH"s80000",
};//PATS_FILE_PATH"s90000"PATS_FILE_PATH"s100000"};*/



int main()
{
	unsigned text_len;//文本大小
	char *text;//文本缓冲区
	char pat[MAXPATLEN + 1];
	int pat_len = 0;
	char *line_break;
	int nfound = 0;
	text_len =0;
	int  len ;
	FILE * file = fopen("myWM2.txt","w");
	//printf("随机产生\n");
 	for(int i = 0; i < sizeof(pats_lsp)/sizeof(char *); i++){
		char *filename =pats_lsp[i];
		p=wmNew();	//创建模式串集

		read_pat_to_wm(p,filename);

		printf("模式集构建好了！模式名称：%s   模式集大小为：%d\t 模式的lsp为：%d\n",filename,p->msNumPatterns, p->msSmallest);
		fprintf(file,"模式集构建好了！模式名称：%s   模式集大小为：%d\t 模式的lsp为：%d\n",filename,p->msNumPatterns, p->msSmallest);
		clock_t start = clock();
		wmPrepPatterns(p);	//对模式串集预处理
		clock_t  end = clock();
		FILE * outfile2 = fopen("hashstatic2.txt","w");
		//检验构建的新的hash表是否是对的
		/*for(int i = 0; i < p->msNumHashEntries; i++){
		if(p->newHash[i]== NULL)
		fprintf(outfile2, "%d%s%s\n",i,"  ","null");
		else{
		if(!p->newHash[i]->isHashTable){
		fprintf(outfile2, "%d%s%s\n",i,"  ","<=20");
		}else{
		fprintf(outfile2, "%d%s%s\n",i,"  ",">20");
		}
		}
		}
		fclose(outfile2);*/


		printf("预处理时间：%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"预处理时间：%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		text = read_text(TEXT_FILE_NAME, &text_len);
		///for()
		len = strlen(text);
		printf("文本串读取结束！！！\n");
		fprintf(file,"文本串读取结束！！！\n");
		start  = clock();

		if (p->msNumPatterns > 0)
		{
			wmSearch(p,(unsigned char *)text,text_len,&nfound);	
		}
		end  =clock();
		fprintf(file,"匹配时间是%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"匹配次数是%d\n",nfound);
		cout << "匹配时间是："<<(double)(end- start)/CLOCKS_PER_SEC << endl;
		cout << "匹配成功的次数："<< nfound<<endl;
	}


	fclose(file);
	system("pause");
	return 0;
}
/*int main()
{

int a;

/*HASH_STRUCT *a = (HASH_STRUCT *)malloc(sizeof(HASH_STRUCT));
a->isHashTable = false;

int  b = 4;
a->HASH_ENTRY.subhashTable = &b;
a->HASH_ENTRY.location = 1;
printf("%d\n", a->isHashTable);
printf("%d\n", a->HASH_ENTRY.location);
printf("%d\n", a->HASH_ENTRY.subhashTable);*/
/*system("pause");
return 0;
}*/