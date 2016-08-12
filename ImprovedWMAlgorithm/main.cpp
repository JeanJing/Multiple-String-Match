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
	return (unsigned short) (((*T)<<8) | *(T+1)); //�Ե�һ���ַ�����8λ��Ȼ����ڶ����ַ��������
}
//str��Ҫhash���ַ�����str_size���ַ����ĳ��ȣ�table_size�Ǳ�
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
	p->msNumPatterns=0;   //ģʽ���ĸ���,��ʼΪ0
	p->msSmallest=1000;   //���ģʽ���ĳ���
	return p;
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
//��������ǰ���ǰ����ַ����ĺ�2���ַ���hashֵ����
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
	//�����µ�hash��
	ps->newHash=(HASH_STRUCT**)malloc(sizeof(HASH_STRUCT*)* ps->msNumHashEntries);	//HASH��
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
			//�ж����hash����Ԫ�ظ���
			int numberOfEntryInSlot = ps->msNumArray[ps->msHash[n]];
			if(numberOfEntryInSlot <= HASHTABLE_THRESHOLD){
				(ps->newHash[n])->isHashTable = false;

				ps->newHash[n]->HASH_ENTRY.location = ps->msHash[n];
			}else{

				ps->newHash[n]->isHashTable = true;
				ps->newHash[n]->HASH_ENTRY.subhashTable = (SUB_HASH_ENTRY **)malloc(sizeof(SUB_HASH_ENTRY*) * numberOfEntryInSlot);
				//����hash��Ҫ����һ���򵥵ĸ�ֵ
				for(int j =0; j< numberOfEntryInSlot; j++)
					ps->newHash[n]->HASH_ENTRY.subhashTable[j] = NULL;
				//���������Ǹ�Ԫ������ӳ�䵽�½���hash����
				for(int j = ps->msHash[n]; j<ps->msHash[n] + numberOfEntryInSlot;j++){
					int slot_number = wiseHash(ps->msPatArray[j].psPat, ps->msSmallest,numberOfEntryInSlot);//���hash�������ǿ���
					//�õ���hash�۵ĺţ����ڵ�һ���жϸò�Ŀǰ��û��Ԫ��
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
static int wmGroupMatch(WM_STRUCT *ps,
						HASH_STRUCT* lindex,//lindexΪ��׺��ϣֵ��ͬ����Щģʽ�Ӵ��е�һ��ģʽ�Ӵ���index,�Ǹ��Ľ�֮�����ֵ����ָ��һ����hash�������hash��һ����������
						unsigned char *Tx,
						unsigned char *T, int numberInHash)
{
	WM_PATTERN_STRUCT *patrn; 
	WM_PATTERN_STRUCT *patrnEnd;
	int text_prefix;
	unsigned char *px,*qx;
	int nfound = 0;
	if(!lindex->isHashTable){
		int index_of_first_pattern = lindex->HASH_ENTRY.location;
		patrn=&ps->msPatArray[index_of_first_pattern];
		patrnEnd=patrn+ps->msNumArray[index_of_first_pattern];

		text_prefix=HASH16(T);


		for(;patrn<patrnEnd;patrn++)
		{
			if(ps->msPrefix[index_of_first_pattern++]!=text_prefix)
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
	}else{
		   
	  int hash_value = wiseHash(T,ps->msSmallest,ps->msNumArray[ps->msHash[numberInHash]]);
	   SUB_HASH_ENTRY* hash_list = lindex->HASH_ENTRY.subhashTable[hash_value];
		text_prefix=HASH16(T);
		while(hash_list!=NULL){
			if(ps->msPrefix[hash_list->location] != text_prefix){
				hash_list= hash_list->next;
				continue;
			}
			else{//���ǰ׺��ͬ�Ϳ�ʼ�ȽϺ�׺
				px=(ps->msPatArray[hash_list->location]).psPat;	//ȡpatrn���ִ�
				qx=T;
				while(*(px++)==*(qx++) && *(qx-1)!='\0');	//����ģʽ�����бȽ�
				if(*(px-1)=='\0')	//ƥ�䵽�˽���λ�ã�˵��ƥ��ɹ�
				{
					//printf("Match pattern \"%s\" at line %d column %d\n",(ps->msPatArray[hash_list->location]).psPat,nline,T-Tx+1);

					nfound++;
				}
				hash_list= hash_list->next;
			}
			
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

		int numberInHash = ((*(window-1)<<8) | *window);

		if((lindex=ps->newHash[numberInHash])==NULL) continue;
		lindex=ps->newHash[numberInHash];//�����ϣֵ����
		found_per =wmGroupMatch(ps,lindex,Tx,T,numberInHash);//��׺��ϣֵ��ͬ���Ƚ�ǰ׺������ģʽ��
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
		terminate("�޷�Ϊ�ı��� ���ڴ�!\n");
	if(fread(text, *text_len, 1, fp_text) != 1)
		fprintf(stderr, "Can not fread file: %s\n", text_file);;
	text[*text_len] = '\0';
	return text;
}




#define TEXT_FILE_NAME "D:/patterns/rand_text500MB" 
#define RAND_TEXT_FILE_NAME "D:/patterns/rand_text.200MB" 
#define PATS_FILE_PATH "D:/patterns/"

char* pats_lsp[] = {PATS_FILE_PATH"r1000000"};/*PATS_FILE_PATH"r3000000",PATS_FILE_PATH"r4000000",
	PATS_FILE_PATH"r5000000",PATS_FILE_PATH"r6000000",PATS_FILE_PATH"r7000000"};/*
	//PATS_FILE_PATH"n30800000",PATS_FILE_PATH"n30900000",PATS_FILE_PATH"n301000000"};
/*char* pats_lsp[] = {PATS_FILE_PATH"s10000", PATS_FILE_PATH"s20000",PATS_FILE_PATH"s30000",PATS_FILE_PATH"s40000",
	PATS_FILE_PATH"s50000",PATS_FILE_PATH"s60000",PATS_FILE_PATH"s70000",PATS_FILE_PATH"s80000",
};//PATS_FILE_PATH"s90000"PATS_FILE_PATH"s100000"};*/



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
	FILE * file = fopen("myWM.txt","w");
	//printf("�������\n");
	for(int i = 0; i < sizeof(pats_lsp)/sizeof(char *); i++){
		char *filename =pats_lsp[i];
		p=wmNew();	//����ģʽ����

		read_pat_to_wm(p,filename);

		printf("ģʽ���������ˣ�ģʽ���ƣ�%s   ģʽ����СΪ��%d\t ģʽ��lspΪ��%d\n",filename,p->msNumPatterns, p->msSmallest);
		fprintf(file,"ģʽ���������ˣ�ģʽ���ƣ�%s   ģʽ����СΪ��%d\t ģʽ��lspΪ��%d\n",filename,p->msNumPatterns, p->msSmallest);
		clock_t start = clock();
		wmPrepPatterns(p);	//��ģʽ����Ԥ����
		clock_t  end = clock();
		FILE * outfile2 = fopen("hashstatic2.txt","w");
		//���鹹�����µ�hash���Ƿ��ǶԵ�
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
		end  =clock();
		fprintf(file,"ƥ��ʱ����%lf\n",(double)(end- start)/CLOCKS_PER_SEC);
		fprintf(file,"ƥ�������%d\n",nfound);
		cout << "ƥ��ʱ���ǣ�"<<(double)(end- start)/CLOCKS_PER_SEC << endl;
		cout << "ƥ��ɹ��Ĵ�����"<< nfound<<endl;
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