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
#define MAXN 10001 //ģʽ������󳤶�MAXN - 1
#define MAXM 51    //������󳤶�ΪMAXM - 1


WM_STRUCT * wmNew()
{
	WM_STRUCT *p=(WM_STRUCT *)malloc(sizeof(WM_STRUCT));
	if(!p) return 0;
	p->msNumPatterns=0;   //ģʽ���ĸ���,��ʼΪ0
	p->msSmallest=1000;   //���ģʽ���ĳ���
	return p;
}


void wmFree(WM_STRUCT *ps) //�ͷſռ亯��
{
	if(ps->msPatArray) //���ģʽ�����д����Ӵ��������ͷ��Ӵ�����ռ�ÿռ�
	{
		if(ps->msPatArray->psPat) free(ps->msPatArray->psPat);	//�Ӵ���Ϊ�գ����ͷ�
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
	WM_PATTERN_STRUCT *p;  //����һ���Ӵ��ṹ
	p=(WM_PATTERN_STRUCT *)malloc(sizeof(WM_PATTERN_STRUCT));
	if(!p) return -1;

	p->psPat=(unsigned char*)malloc(m+1); //���Ӵ�����ĳ��ȷ���ռ�
	memset(p->psPat+m,0,1);	//���һ��λ������Ϊ�����ַ���/0�� 
	memcpy(p->psPat,q,m); //����q���Ӵ��ṹ������
	p->psLen=m;           //�Ӵ����ȸ�ֵ
	ps->msNumPatterns++; //ģʽ�������Ӵ�������1
	if(p->psLen < (unsigned)ps->msSmallest) ps->msSmallest = p->psLen; //����ȷ������ַ�������

	p->next=ps->plist; //�������Ӵ������ַ������б��С�������ʽ�������ڶ���ͷ��
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
	return (unsigned short) (((*T)<<8) | *(T+1)); //�Ե�һ���ַ�����8λ��Ȼ����ڶ����ַ��������
}

void sort(WM_STRUCT *ps)
{
	int m=ps->msSmallest; //��ȡ����Ӵ�����
	int i,j;
	unsigned char *temp;
	int flag;	//ð������ı�־λ����һ�˱Ƚ��޽���ʱ��˵���Ѿ�������򣬼���������ѭ������
	for(i = ps->msNumPatterns-1,flag = 1;i > 0 && flag;i--)  //ѭ�����ַ������е�ÿ���Ӵ����������ϣֵ��С����ð������
	{
		flag=0;
		for(j=0;j<i;j++)
		{
			if(HASH16(&(ps->msPatArray[j+1].psPat[m-2]))<HASH16(&(ps->msPatArray[j].psPat[m-2])))//�Ƚϵ�Ϊÿ���Ӵ���ȡ���ֵ���������ַ��Ĺ�ϣֵ
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
	ps->msNumHashEntries=HASHTABLESIZE;	//HASH��Ĵ�С
	ps->msHash=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)* ps->msNumHashEntries);	//HASH��
	if(!ps->msHash)
	{
		printf("No memory in wmPrepHashedPatternGroups()\n");
		return;
	}

	for(i=0;i<(int)ps->msNumHashEntries;i++)	//HASH��Ԥ�����ʼ����ȫ����ʼ��Ϊ(HASH_TYPE)-1
	{
		ps->msHash[i]=(HASH_TYPE)-1;
	}
	//FILE * outfile = fopen("hashstatic.txt","w");
	for(i=0;i<ps->msNumPatterns;i++)	//��������Ӵ�����HASHԤ����
	{
		hindex=HASH16(&ps->msPatArray[i].psPat[m-2]);	//��ģʽ�Ӵ�����������ַ������ϣֵ��ƥ�䣩
		sindex=ps->msHash[hindex]=i;
		ningroup=1;
		//��ʱ��ϣ���Ѿ�������
		while((++i<ps->msNumPatterns) && (hindex==HASH16(&ps->msPatArray[i].psPat[m-2])))	//�Һ�׺��ͬ���Ӵ���
			ningroup++;
		ps->msNumArray[sindex]=ningroup;	//��i���Ӵ���������ģʽ�������׺2�ַ���ͬ�Ӵ��ĸ���
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

	for(i=0;i<SHIFTTABLESIZE;i++)	//��ʼ��Shift����ʼֵΪ����ַ����ĳ���
	{
		ps->msShift[i]=(unsigned)(m-2+1);
	}

	for(i=0;i<ps->msNumPatterns;i++)	//���ÿ���Ӵ�Ԥ����
	{
		for(k=0;k<m-1;k++)
		{
			shift=(unsigned short)(m-2-k);
			cindex=((ps->msPatArray[i].psPat[k]<<8) | (ps->msPatArray[i].psPat[k+1]));//BΪ2
			if(shift < ps->msShift[cindex])
				ps->msShift[cindex] = shift;//k=m-2ʱ��shift=0��
		}
	}
}

static void wmPrepPrefixTable(WM_STRUCT *ps)//����Prefix��
{
	int i;
	ps->msPrefix=(HASH_TYPE*)malloc(sizeof(HASH_TYPE)* ps->msNumPatterns);	//����ռ䳤��Ϊ�����Ӵ��ĸ���*
	if(!ps->msPrefix)
	{
		printf("No memory in wmPrepPrefixTable()\n");
		return;
	}

	for(i=0;i<ps->msNumPatterns;i++)	//��ϣ����Prefix��
	{
		ps->msPrefix[i]=HASH16(ps->msPatArray[i].psPat);//��ÿ��ģʽ����ǰ׺���й�ϣ
	}
} 


static int wmGroupMatch(WM_STRUCT *ps,
						int lindex,//lindexΪ��׺��ϣֵ��ͬ����Щģʽ�Ӵ��е�һ��ģʽ�Ӵ���index
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
		else	//�����׺��ϣֵ��ͬ����
		{
			px=patrn->psPat;	//ȡpatrn���ִ�
			qx=T;
			while(*(px++)==*(qx++) && *(qx-1)!='\0');	//����ģʽ�����бȽ�
			if(*(px-1)=='\0')	//ƥ�䵽�˽���λ�ã�˵��ƥ��ɹ�
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

	ps->msNumArray=(unsigned short*)calloc(sizeof(short),ps->msNumPatterns);//�����һ���̫���
	if(!ps->msNumArray)
		return -1;

	for(kk=0,plist=ps->plist;plist!=NULL && kk<ps->msNumPatterns;plist=plist->next)
	{
		memcpy(&ps->msPatArray[kk++],plist,sizeof(WM_PATTERN_STRUCT));
	}
	static int m = ps->msSmallest;
	//sort(ps);
	qsort(ps->msPatArray,ps->msNumPatterns,sizeof(WM_PATTERN_STRUCT),sortcmp);	//��ϣ����


	wmPrepHashedPatternGroups(ps);	//��ϣ��  
	wmPrepShiftTable(ps);	//shift��
	wmPrepPrefixTable(ps);	//Prefix��
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
	Tleft=n;//���Ӧ���Ǳ�ʾ��ʣ�¼����ַ�û�б��ȽϹ�
	Tend=Tx+n;
	if(n < ps->msSmallest)	/*�����ҵ��ַ������б���Сģʽ�Ӵ����̣�
							��Ȼ�ǲ����ܲ��ҵ��ģ�ֱ���˳�*/
							return;
	//window�൱�ڴ��ڵ�ĩβ
	for(T=Tx,window=Tx+ps->msSmallest-1;window<Tend;T++,window++,Tleft--)
	{
		tshift = ps->msShift[(*(window-1)<<8) | *window];
		while(tshift)//��tshift!=0,��ƥ�䣬�Ǿ͸��������תֵ�����ת��ֱ�����ҵ���תֵΪ0����
		{
			window+=tshift;
			T+=tshift;
			Tleft-=tshift;
			if(window>Tend) return;
			tshift=ps->msShift[(*(window-1)<<8) | *window];
		}
		//tshift=0��������׺��ϣֵ�Ѿ���ͬ
		if((lindex=ps->msHash[(*(window-1)<<8) | *window])==-1) continue;
		lindex=ps->msHash[(*(window-1)<<8) | *window];//�����ϣֵ����
		found_per =wmGroupMatch(ps,lindex,Tx,T);//��׺��ϣֵ��ͬ���Ƚ�ǰ׺������ģʽ��
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
	unsigned text_len;//�ı���С
	char *text;//�ı�������
	char pat[MAXPATLEN + 1];
	int pat_len = 0;
	char *line_break;
	int nfound = 0;
	text_len =0;
	int  len ;
	FILE * file = fopen("WM.txt","w");

	printf("�������\n");
	for(int i = 0; i < sizeof(pats_lsp)/sizeof(char *); i++){
		char *filename =pats_lsp[i];
		p=wmNew();	//����ģʽ����

		read_pat_to_wm(p,filename);

		printf("ģʽ���������ˣ�ģʽ���ƣ�%s   ģʽ����СΪ��%d\t ģʽ��lspΪ��%d\n",filename,p->msNumPatterns, p->msSmallest);
		fprintf(file,"ģʽ���������ˣ�ģʽ���ƣ�%s   ģʽ����СΪ��%d\t ģʽ��lspΪ��%d\n",filename,p->msNumPatterns, p->msSmallest);
		clock_t start = clock();
		wmPrepPatterns(p);	//��ģʽ����Ԥ����
		clock_t  end = clock();
		printf("Ԥ����ʱ�䣺%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"Ԥ����ʱ�䣺%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		text = read_text(TEXT_FILE_NAME, &text_len);
		///for()
		len = strlen(text);
		printf("�ı�����ȡ����������\n");
		fprintf(file,"�ı�����ȡ����������\n");
		start  = clock();

		if (p->msNumPatterns > 0)
		{
			wmSearch(p,(unsigned char *)text,text_len,&nfound);	
		}
		end = clock();
		fprintf(file,"ƥ��ʱ����%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"ƥ�������%d\n",nfound);
		printf("ƥ��ʱ�䣺%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		printf("%d\n",nfound);
		wmFree(p);
		//getchar();
	}

	system("pause");
	return(0);
}