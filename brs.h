#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//const int MAX_K=32;
#define MAX_K 32
#define BYTE unsigned char 
//#define DT char
//#define DT short
#define DT long
//#define DT long long
//#define DT __int64

// a = a xor (b<<offset)
// there are 'length' BYTEs of significative datas in both 'a' and 'b' .
// The array 'a' must has a size of " length + offset " BYTEs
void leftxor(BYTE *a,const BYTE *b,int offset,int length){
	register DT * ia=(DT*)(&a[offset]);
	register DT * ib=(DT*)b;
	int l4=length/sizeof(DT);
	register int i;
	//for(i=0;i<l4;++i){

	for(i=l4;i>0;--i){
		*(ia++)^=*(ib++);
	//	++ia;++ib;
	}
}

// The main decode method
// this function is used to solve the problem:
//		coding = repair * mc
//		coding[i] = xor(0<=i<numRepairO){ repair[j] << mc[i][j] }
//		each array has "blocksize_DT" BYTEs of significative datas 
// all the parameters are known, except that "repair" is the wanted output!
void ker_decode(int numRepairO,DT *coding[],DT * repair[],int mc[MAX_K][MAX_K],int max_blocksize_DT,int blocksize_DT){
	int i,j;
	int s[MAX_K]={0},mcj[MAX_K]={0};
	
	if(numRepairO>6) {
	//this is the basic case which numRepairO can be any positive integer.
	//If numRepairO is too small, the code can be expanded to make a high efficiency.
		DT *src2[MAX_K]={0};
		s[0]=0;
		for(i=1;i<numRepairO;++i)
			s[i]=s[i-1]+mc[numRepairO-i-1][i]-mc[numRepairO-i-1][i-1];
		int j;
		for(j=0;j<numRepairO;++j) {
			src2[j]=coding[numRepairO-1-j];
			mcj[j]=mc[numRepairO-1-j][j];
		}
		for(i=0;i<max_blocksize_DT;++i){
			for(j=0;j<numRepairO;++j)
			{
				int aj=i-s[j];
				if(aj>=0 && aj<blocksize_DT) 
				{
					DT t=src2[j][mcj[j]+aj];
					repair[j][aj]=t;
					int i2;
					for(i2=0;i2<numRepairO;++i2)
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}
	}else if(numRepairO==1)	{
		memcpy(repair[0],&coding[0][mc[0][0]],blocksize_DT*sizeof(DT));
	}else if(numRepairO==2)	{
		int d=mc[0][1]-mc[0][0];
		DT *c00,*c01,*c11,*c10;
		int i;
		c00=&coding[0][mc[0][0]];
		c10=&coding[1][mc[1][0]];
		c11=&coding[1][mc[1][1]-d];
		c01=&coding[0][mc[0][1]-d];
		for(i=0;i<d;++i){
			*c00^=*c10;
			c00++;c10++;c11++;c01++;
		}
		for(i=d;i<blocksize_DT;++i){
			*(c00++)^=*(c10++);
			*(c11++)^=*(c01++);
		}
		for(i=blocksize_DT;i<blocksize_DT+d;++i){
			*c11^=*c01;
			c00++;c10++;c11++;c01++;
		}
		memcpy(repair[0],&coding[1][mc[1][0]],blocksize_DT*sizeof(DT));
		memcpy(repair[1],&coding[0][mc[0][1]],blocksize_DT*sizeof(DT));
	}else if(numRepairO==3)	{
#define R 3
		s[0]=0;
		for(i=1;i<R;++i) 
			s[i]=s[i-1]+mc[R-i-1][i]-mc[R-i-1][i-1];

		for(i=0;i<s[R-1];++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj>=0){
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}

		DT *c[R][R]={0};
		for(i=0;i<R;i++){
			for(j=0;j<R;j++){
				c[i][j]=&coding[j][mc[j][i]+s[R-1]-s[i]];
			}
		}

		for(i=s[R-1];i<blocksize_DT;++i){
			DT t;
			t=*(c[0][2]++);     *(c[0][0]++)^=t;*(c[0][1]++)^=t;
			t=*(c[1][1]++);     *(c[1][0]++)^=t;                *(c[1][2]++)^=t;
			t=*(c[2][0]++);                     *(c[2][1]++)^=t;*(c[2][2]++)^=t;
		}

		for(i=blocksize_DT;i<max_blocksize_DT;++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj<blocksize_DT) {
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}
		for(i=0;i<R;i++)
		memcpy(repair[i],&coding[R-1-i][mc[R-1-i][i]],blocksize_DT*sizeof(DT));
#undef R
	}else if(numRepairO==4) {
#define R 4
		s[0]=0;
		for(i=1;i<R;++i) 
			s[i]=s[i-1]+mc[R-i-1][i]-mc[R-i-1][i-1];

		for(i=0;i<s[R-1];++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj>=0){
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for( i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}

		DT *c[R][R]={0};
		for(i=0;i<R;i++){
			for(j=0;j<R;j++){
				c[i][j]=&coding[j][mc[j][i]+s[R-1]-s[i]];
			}
		}

		for(i=s[R-1];i<blocksize_DT;++i){
			DT t;
			t=*(c[0][3]++);     *(c[0][0]++)^=t;*(c[0][1]++)^=t;*(c[0][2]++)^=t;
			t=*(c[1][2]++);     *(c[1][0]++)^=t;*(c[1][1]++)^=t;                *(c[1][3]++)^=t;
			t=*(c[2][1]++);     *(c[2][0]++)^=t;                *(c[2][2]++)^=t;*(c[2][3]++)^=t;
			t=*(c[3][0]++);                     *(c[3][1]++)^=t;*(c[3][2]++)^=t;*(c[3][3]++)^=t;
		}

		for(i=blocksize_DT;i<max_blocksize_DT;++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj<blocksize_DT) {
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for( i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}
		for(i=0;i<R;i++)
		memcpy(repair[i],&coding[R-1-i][mc[R-1-i][i]],blocksize_DT*sizeof(DT));
#undef R
	}else if(numRepairO==5) {
#define R 5
		s[0]=0;
		for(i=1;i<R;++i) 
			s[i]=s[i-1]+mc[R-i-1][i]-mc[R-i-1][i-1];

		for(i=0;i<s[R-1];++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj>=0){
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}

		DT *c[R][R]={0};
		for(i=0;i<R;i++){
			for(j=0;j<R;j++){
				c[i][j]=&coding[j][mc[j][i]+s[R-1]-s[i]];
			}
		}

		for(i=s[R-1];i<blocksize_DT;++i){
			DT t;
			t=*(c[0][4]++);     *(c[0][0]++)^=t;*(c[0][1]++)^=t;*(c[0][2]++)^=t;*(c[0][3]++)^=t;    
			t=*(c[1][3]++);     *(c[1][0]++)^=t;*(c[1][1]++)^=t;*(c[1][2]++)^=t;                *(c[1][4]++)^=t;
			t=*(c[2][2]++);     *(c[2][0]++)^=t;*(c[2][1]++)^=t;                *(c[2][3]++)^=t;*(c[2][4]++)^=t;
			t=*(c[3][1]++);     *(c[3][0]++)^=t;                *(c[3][2]++)^=t;*(c[3][3]++)^=t;*(c[3][4]++)^=t;
			t=*(c[4][0]++);                     *(c[4][1]++)^=t;*(c[4][2]++)^=t;*(c[4][3]++)^=t;*(c[4][4]++)^=t;
		}

		for(i=blocksize_DT;i<max_blocksize_DT;++i){
			for(j=0;j<R;j++){
				int aj=i-s[j];
				if(aj<blocksize_DT) {
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}
		for(i=0;i<R;i++)
		memcpy(repair[i],&coding[R-1-i][mc[R-1-i][i]],blocksize_DT*sizeof(DT));
#undef R
	}else if(numRepairO==6) {
#define R 6
		s[0]=0;
		for(i=1;i<R;++i) 
			s[i]=s[i-1]+mc[R-i-1][i]-mc[R-i-1][i-1];

		for(i=0;i<s[R-1];++i){
			for( j=0;j<R;j++){
				int aj=i-s[j];
				if(aj>=0){
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}

		DT *c[R][R]={0};
		for(i=0;i<R;i++){
			for( j=0;j<R;j++){
				c[i][j]=&coding[j][mc[j][i]+s[R-1]-s[i]];
			}
		}

		for(i=s[R-1];i<blocksize_DT;++i){
			DT t;
			t=*(c[0][5]++);     *(c[0][0]++)^=t;*(c[0][1]++)^=t;*(c[0][2]++)^=t;*(c[0][3]++)^=t;*(c[0][4]++)^=t;                
			t=*(c[1][4]++);     *(c[1][0]++)^=t;*(c[1][1]++)^=t;*(c[1][2]++)^=t;*(c[1][3]++)^=t;                *(c[1][5]++)^=t;
			t=*(c[2][3]++);     *(c[2][0]++)^=t;*(c[2][1]++)^=t;*(c[2][2]++)^=t;                *(c[2][4]++)^=t;*(c[2][5]++)^=t;
			t=*(c[3][2]++);     *(c[3][0]++)^=t;*(c[3][1]++)^=t;                *(c[3][3]++)^=t;*(c[3][4]++)^=t;*(c[3][5]++)^=t;
			t=*(c[4][1]++);     *(c[4][0]++)^=t;                *(c[4][2]++)^=t;*(c[4][3]++)^=t;*(c[4][4]++)^=t;*(c[4][5]++)^=t;
			t=*(c[5][0]++);                     *(c[5][1]++)^=t;*(c[5][2]++)^=t;*(c[5][3]++)^=t;*(c[5][4]++)^=t;*(c[5][5]++)^=t;   
		}

		for(i=blocksize_DT;i<max_blocksize_DT;++i){
			for( j=0;j<R;j++){
				int aj=i-s[j];
				if(aj<blocksize_DT) {
					DT t=coding[R-1-j][mc[R-1-j][j]+aj];
					int i2;
					for(i2=0;i2<R;i2++) if(i2+j!=R-1)	
						coding[i2][mc[i2][j]+aj]^=t;
				}
			}
		}
		for(i=0;i<R;i++)
		memcpy(repair[i],&coding[R-1-i][mc[R-1-i][i]],blocksize_DT*sizeof(DT));
#undef R
	}
}


int SECTION_SIZE2(int kValue, int nValue, int pagesize){
	int L = pagesize;
	while(L % (kValue*sizeof(DT)) != 0) L++;
	L /= kValue;
	return L+(nValue-kValue-1)*(kValue-1)*sizeof(DT);
}

int SECTION_SIZE(int kValue, int nValue, int pagesize, int site){
	int L = pagesize;
	while(L % (kValue*sizeof(DT)) != 0) L++;
	L /= kValue;
	
	if(site < 0 || site >= nValue){
		printf("Error, site should be > 0 and < nvalue\n");
		return -1;
	}
	if( site < kValue)
		return L;
	else  
		return L+(site-kValue)*(kValue-1)*sizeof(DT);
}

int encode_page_to_section_bysection(BYTE nValue,BYTE kValue, BYTE *section_buf[], BYTE *shm_buf,int pagesize){
	int i,j;
	int blocksize = pagesize;
	int max_blocksize;

	if(blocksize%(kValue*sizeof(DT)) != 0){printf("pagesize is invalid: blocksize mod (kValue*%d) is not 0 \n",(int) sizeof(DT));return -1;}
	//while(blocksize % (kValue*sizeof(DT)) != 0) blocksize++;

	blocksize /= kValue;  //count the actual blocksize
	max_blocksize = blocksize + (kValue-1)*(nValue-kValue-1)*sizeof(DT);

	//generate data blocks
	for(i = 0;i < kValue;i++){
		//memset(section_buf[i],0,max_blocksize);
	//	memset(((BYTE*)section_buf[i])+blocksize,0,max_blocksize-blocksize);
		section_buf[i] = &shm_buf[i*blocksize];

	}

	//generate parity blocks
	for(j = kValue;j < nValue;j++){
		memcpy(section_buf[j],section_buf[0],max_blocksize);
		for (i = 1;i < kValue;i++){
			leftxor(section_buf[j],section_buf[i],(j-kValue)*i*sizeof(DT),blocksize);
		}
	}
	return 0;
}

int encode_page_to_section_bybyte(BYTE nValue,BYTE kValue, BYTE *section_buf[], BYTE *shm_buf, int pagesize){
	return encode_page_to_section_bysection(nValue,kValue, section_buf, shm_buf,pagesize);
}
int decode_section_to_page_bysection(BYTE nValue, BYTE kValue, int result[], BYTE *section_buf[], BYTE *shm_buf, int pagesize){
	int i,j;
	int goodCode=0;
	int numRepairO=0;
	int cexist[MAX_K]={0};
	int olost[MAX_K]={0};
	int mc[64][MAX_K]={0};
	DT * coding[MAX_K]={0};
	DT * repair[MAX_K]={0};
	int rc=0;
	int blocksize = pagesize;
	if(blocksize%(kValue*sizeof(DT)) != 0){printf("pagesize is invalid: blocksize mod (kValue*%d) is not 0 \n",(int) sizeof(DT));return -1;}
	while(blocksize % (kValue*sizeof(DT)) != 0) blocksize++;
	blocksize /= kValue;
	int max_blocksize_DT=blocksize/sizeof(DT)+(kValue-1)*(nValue-kValue-1)*2;

	//check the result[]
	for (i=0;i<nValue-kValue;i++){	
		if (result[i+kValue]!=0){
			cexist[goodCode++]=i;
		}
	}
	for (i=0;i<kValue;i++){		
		if (result[i]==0){
			olost[numRepairO++]=i;
		}
	}
	//check if this situation can be decoded
	if(goodCode<numRepairO) {
		printf("There are too much files broken, and could not repair.\n");
		return -1;
	}
	//generate the repair matrix
	for (i=0;i<numRepairO;i++)
	{
		for(j=0;j<numRepairO;++j){
			mc[i][j]=cexist[i]*olost[j];
		}
	}

	//\BD\AB\CD\EA\BA\C3\CE\DE\CB\F0\B5\C4\CA\FD\BEݿ\E9д\C8\EBshm_buf\A3\AC\B2\A2\C7\D2\C9\E8\D6\C3Ҫ\D0޸\B4\B5\C4\CA\FD\BEݿ飬\B2\A2\C7\D2\D4\DA\D0޸\B4ʱֱ\BD\D3д\B5\BDshm_buf\D6\D0
	//copy the existing data blocks to the shm_buf, and set the location of the failed data blocks when fixing.
	for (i=0;i<kValue;i++)
	{
		if(result[i]==0) {
			//repair[rc++]=(DT*)section_buf[i];
			repair[rc++]=(DT*)(&shm_buf[i*blocksize]);
		}else{
			memcpy(&shm_buf[i*blocksize],section_buf[i],blocksize);
		}
	}

	//remove all the existing data blocks from the parity/coding blocks.
	for (j=0;j<numRepairO;j++)
	{
		int cline=cexist[j];
		coding[j]=(DT*)section_buf[cexist[j]+kValue];
		for (i=0;i<kValue;i++)
		{
			if(result[i]!=0)
				leftxor((BYTE*)coding[j],(BYTE*)section_buf[i],cline*i*sizeof(DT),blocksize);
		}
	}
	//run the ker_decode function to solve all the failed data blocks. 
	ker_decode(numRepairO,coding,repair,mc,max_blocksize_DT,blocksize/sizeof(DT));
	return 0;
}
int decode_section_to_page_bybyte(BYTE nValue, BYTE kValue, int result[], BYTE *section_buf[], BYTE *shm_buf,int pagesize){
	return decode_section_to_page_bysection(nValue, kValue, result, section_buf, shm_buf, pagesize);
}
//#undef DT

