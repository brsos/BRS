#include"BRSCoder.h"
#include"timer.h"

// generate a matrix with "a" rows, and "len" colomns, with the type of each element is BYTE
BYTE** getMatr(int a,int len){
	BYTE** res=new BYTE*[a];
	for(int i=0;i<a;i++){
		res[i]=new BYTE[len];
		memset(res[i],0,len*sizeof(BYTE));
	}
	return res;
}
void print(BYTE** mat,int r,int c){
	for(int i=0;i<r;i++){
		for(int j=0;j<c;j++){
			printf("%02x ",(int)mat[i][j]&255);
		}
		printf("\n");
	}	
}

void XorAll(BYTE *a,const BYTE *b,int length){
	register long * ia=(long*)a;
	register long * ib=(long*)b;
	int l4=length/sizeof(long);
	register int i;
	//for(i=0;i<l4;++i){

	for(i=l4;i>0;--i){
		*(ia++)^=*(ib++);
	}
}

// data has k+m+l blocks,:
// 0~k-1 is for origin data blocks,
// k~k+m-1 is for global parity blocks,
// k+m~k+m+l-1 is for local parity blocks
void LRC_encode(int k,int m,int l,BYTE ** data,int len){
	encode(k,m,data,len);

	if(l<=1) return;
	for(int i=0;i<k;i++){
		if(i<l){
			memcpy(data[k+m+i],data[i],len);
		}else{
			XorAll(data[k+m+i%l],data[i],len);
		}
	}
}

// data has k+m+l blocks, so as the flag,
// 0~k-1 is for origin data blocks,
// k~k+m-1 is for global parity blocks,
// k+m~k+m+l-1 is for local parity blocks
void LRC_decode(int k,int m,int l,BYTE ** data,int flag[],int len){
	int i;
	if(l>1){
		for(i=k+m;i<k+m+l;i++){
			if(flag[i]){
				int lost=0;
				int lastId=-1;
				for(int j=i-k-m;j<k;j+=l){
					if(flag[j]==0) {lost++;lastId=j;}
				}
				if(lost==1){
					memcpy(data[lastId],data[i],len);
					for(int t=i-k-m;t<k;t+=l){
						if(t!=lastId) XorAll(data[lastId],data[t],len);
					}
					flag[lastId]=1;
				}
			}
		}
	}
	int finish=1;
	for(i=0;i<k;i++) if(flag[i]==0) finish=0;//printf("%d %d\n",i,flag[i]);
	if(finish) return;
	decode(k,m,data,flag,len);
	for(i=0;i<k;i++) flag[i]=1;
}

// fill an array of size n element with 1. then randomly set some elements with 0, and the number of 0 is lostNum .
void randomLost(int flag[],int lostNum,int n){
	if(lostNum>=n) return;
	for(int t=0;t<n;t++) flag[t]=0;
	for(int i=0;i<lostNum;i++){
		int a;
		do{
			a=rand()%n;
		}while(flag[a]==0);
		flag[a]=0;
	}
}


int k=8,m=4,l=2,len=1024;
int cycleTime=10000;
int lostNum=2;
int threadsNum=1;

int run();

int main(int argc,char** argv){
	if(argc<8) {
		printf("brs blockSize cycleTime K M L lostNum threadsNum\n");
		printf("L is for the number of Local Parity Blocks in LRC\n");
		return 1;
	}else{
		len=atoi(argv[1]);
		cycleTime=atoi(argv[2]);
		k=atoi(argv[3]);
		m=atoi(argv[4]);
		l=atoi(argv[5]);
		lostNum=atoi(argv[6]);
		threadsNum=atoi(argv[7]);
		//*/
	}
#pragma omp parallel for
	for(int i = 0; i < threadsNum; i++){
		run();
	}
	return 0;
}

int run(){
	int n=k+m+l;
	double start,end;
	BYTE ** data=getMatr(n,len);
	int *flag=new int[n];
	int dl=dataBlockSize(k,m,len);
	int i,j;
	for(i=0;i<k;i++){
		for(j=0;j<dl;j++){
			data[i][j]=(i+1)*(j+1);
		}
	}	
	for(i=0;i<n;i++) flag[i]=1;

	initial(k,m);
	
	start=getTime();
	for(i=0;i<cycleTime;i++){
		LRC_encode(k,m,l,data,len);
	}
	end=getTime();
	printf("encodeTime(s):\t%f\tencode Speed(MB/s):\t%f\n",end-start,(m+l)/(end-start)*dl*cycleTime/(1024*1024));

	
	srand((int)start);
	start=getTime();
	for(i=0;i<cycleTime;i++){
		randomLost(flag,lostNum,k);
		//for(int t=0;t<k;t++) if(flag[t]==0) printf("%d ",t);printf("\n");
		LRC_decode(k,m,l,data,flag,len);
	}
	end=getTime();

	printf("decodeTime(s):\t%f\tdecode Speed(MB/s):\t%f\n",end-start,lostNum/(end-start)*dl*cycleTime/(1024*1024));

	release();
	return 0;
}
