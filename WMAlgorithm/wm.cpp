#include "wm.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "add.h"
#include "config.h"
#include <iostream>
#include <stdio.h>
using namespace std;

//extern int nline=1;

int nline=1;
#define MAXN 10001 //模式串的最大长度MAXN - 1
#define MAXM 51    //单词最大长度为MAXM - 1


WM_STRUCT * wmNew()
{
	WM_STRUCT *p=(WM_STRUCT *)malloc(sizeof(WM_STRUCT));
	if(!p) return 0;
	p->msNumPatterns=0;   //模式串的个数,初始为0
	p->msSmallest=1000;   //最短模式串的长度
	return p;
}


void wmFree(WM_STRUCT *ps) //释放空间函数
{
	if(ps->msPatArray) //如果模式串集中存在子串，则先释放子串数组占用空间
	{
		if(ps->msPatArray->psPat) free(ps->msPatArray->psPat);	//子串不为空，则释放
		free(ps->msPatArray );
	}
	if(ps->msNumArray) free(ps->msNumArray);
	if(ps->msHash) free(ps->msHash);
	if(ps->msPrefix) free(ps->msPrefix);
	if(ps->msShift) free(ps->msShift);
	free(ps);
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

void sort(WM_STRUCT *ps)
{
	int m=ps->msSmallest; //获取最短子串长度
	int i,j;
	unsigned char *temp;
	int flag;	//冒泡排序的标志位。当一趟比较无交换时，说明已经完成排序，即可以跳出循环结束
	for(i = ps->msNumPatterns-1,flag = 1;i > 0 && flag;i--)  //循环对字符串集中的每个子串，根据其哈希值大小进行冒泡排序
	{
		flag=0;
		for(j=0;j<i;j++)
		{
			if(HASH16(&(ps->msPatArray[j+1].psPat[m-2]))<HASH16(&(ps->msPatArray[j].psPat[m-2])))//比较的为每个子串截取部分的最后两个字符的哈希值
			{
				flag=1;
				temp=ps->msPatArray[j+1].psPat;
				ps->msPatArray[j+1].psPat=ps->msPatArray[j].psPat;
				ps->msPatArray[j].psPat=temp;
			}
		}
	}
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
	//fclose(outfile);
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


static int wmGroupMatch(WM_STRUCT *ps,
						int lindex,//lindex为后缀哈希值相同的那些模式子串中的一个模式子串的index
						unsigned char *Tx,
						unsigned char *T)
{
	WM_PATTERN_STRUCT *patrn; 
	WM_PATTERN_STRUCT *patrnEnd;
	int text_prefix;
	unsigned char *px,*qx;
	int nfound = 0;

	patrn=&ps->msPatArray[lindex];
	patrnEnd=patrn+ps->msNumArray[lindex];

	text_prefix=HASH16(T);


	for(;patrn<patrnEnd;patrn++)
	{
		if(ps->msPrefix[lindex++]!=text_prefix)
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
	return nfound;
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
int  sortcmp( const void * e1, const void * e2 )
{
	WM_PATTERN_STRUCT *r1= (WM_PATTERN_STRUCT*)e1;
	WM_PATTERN_STRUCT *r2= (WM_PATTERN_STRUCT*)e2;
	int ms_smallest = get_ms_smallest();
	return (HASH16(&(r1->psPat[ms_smallest-2])) - HASH16(&(r2->psPat[ms_smallest-2])));
}
int bcompare( unsigned char *a, int alen, unsigned char * b, int blen ) 
{
	int stat;
	if( alen == blen )
	{
		return memcmp(a,b,alen);
	}
	else if( alen < blen )
	{
		if( (stat=memcmp(a,b,alen)) != 0 ) 
			return stat;
		return -1;
	}
	else 
	{
		if( (stat=memcmp(a,b,blen)) != 0 ) 
			return stat;
		return +1;
	}
}
void wmSearch(WM_STRUCT *ps,unsigned char *Tx,int n,int *nfound)
{
	*nfound = 0;
	int Tleft,lindex,tshift;
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
		if((lindex=ps->msHash[(*(window-1)<<8) | *window])==-1) continue;
		lindex=ps->msHash[(*(window-1)<<8) | *window];//如果哈希值存在
		found_per =wmGroupMatch(ps,lindex,Tx,T);//后缀哈希值相同，比较前缀及整个模式串
		(*nfound) = (*nfound) +found_per;
	}
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

static WM_STRUCT *p;  
int get_ms_smallest(){
	return p->msSmallest;
}
#define TEXT_FILE_NAME "D:/patterns/rand_text500MB" 
#define RAND_TEXT_FILE_NAME "D:/patterns/rand_text.200MB" 
#define PATS_FILE_PATH "D:/patterns/"
char* pats_lsp[] = {PATS_FILE_PATH"r10000",PATS_FILE_PATH"r20000",PATS_FILE_PATH"r30000",PATS_FILE_PATH"r40000",
	PATS_FILE_PATH"r50000",PATS_FILE_PATH"r60000",PATS_FILE_PATH"r70000",
	PATS_FILE_PATH"80000",PATS_FILE_PATH"90000",PATS_FILE_PATH"100000"};
char* pats_size[] = {PATS_FILE_PATH"english_2.05G.200MB_5_50_10w"};/*,PATS_FILE_PATH"english_2.05G.200MB_5_50_20w",PATS_FILE_PATH"english_2.05G.200MB_5_50_30w",PATS_FILE_PATH"english_2.05G.200MB_5_50_40w",
																   PATS_FILE_PATH"english_2.05G.200MB_5_50_50w",PATS_FILE_PATH"english_2.05G.200MB_5_50_60w",PATS_FILE_PATH"english_2.05G.200MB_5_50_70w",PATS_FILE_PATH"english_2.05G.200MB_5_50_80w",
																   PATS_FILE_PATH"english_2.05G.200MB_5_50_90w"};*/

/*char* rand_extract_pats_lsp[] = {PATS_FILE_PATH"rand_text.200MB_extract_10w_4_50",PATS_FILE_PATH"200000",PATS_FILE_PATH"300000",PATS_FILE_PATH"rand_text.200MB_extract_10w_5_50",
PATS_FILE_PATH"rand_text.200MB_extract_10w_6_50",PATS_FILE_PATH"rand_text.200MB_extract_10w_7_50",PATS_FILE_PATH"rand_text.200MB_extract_10w_8_50",
PATS_FILE_PATH"rand_text.200MB_extract_10w_9_50"};
char* rand_produce_pats_size[] = {PATS_FILE_PATH"100000",PATS_FILE_PATH"200000",PATS_FILE_PATH"300000",PATS_FILE_PATH"produce_10w_5_50",
PATS_FILE_PATH"produce_10w_6_50",PATS_FILE_PATH"produce_10w_7_50",PATS_FILE_PATH"produce_10w_8_50",PATS_FILE_PATH"produce_10w_9_50"};
*/

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
	FILE * file = fopen("WM.txt","w");

	printf("随机产生\n");
	for(int i = 0; i < sizeof(pats_lsp)/sizeof(char *); i++){
		char *filename =pats_lsp[i];
		p=wmNew();	//创建模式串集

		read_pat_to_wm(p,filename);

		printf("模式集构建好了！模式名称：%s   模式集大小为：%d\t 模式的lsp为：%d\n",filename,p->msNumPatterns, p->msSmallest);
		fprintf(file,"模式集构建好了！模式名称：%s   模式集大小为：%d\t 模式的lsp为：%d\n",filename,p->msNumPatterns, p->msSmallest);
		clock_t start = clock();
		wmPrepPatterns(p);	//对模式串集预处理
		clock_t  end = clock();
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
		end = clock();
		fprintf(file,"匹配时间是%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"匹配次数是%d\n",nfound);
		printf("匹配时间：%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		printf("%d\n",nfound);
		wmFree(p);
		//getchar();
	}

	system("pause");
	return(0);
}