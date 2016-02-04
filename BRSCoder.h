#include"brs.h"

void initial(int k, int m){

}
void release(){

}
/*
input:
sz: the maximum length of a block.
output:
data Block Size, the significative data length in the block.
 */
int dataBlockSize(int k, int m, int sz){
	sz-=(k-1)*(m-1)*sizeof(DT);
	if(sz<0) sz=0;
	return sz;
};

/*
input:
section_buf: a 2-dimemsion array, with (k+m) rows, sz columns.
	while the 0 ~ k-1 rows are the original data which is read only.
	and the k ~ k+m-1 rows are the coding data which will be update as the final result.
 */
void encode(int k, int m, BYTE** section_buf, int sz){
	sz-=(k-1)*(m-1)*sizeof(DT);	if(sz<=0) return;
	int i,j;
	for(j = k;j < k+m;j++){
		memcpy(section_buf[j],section_buf[0],sz);
		for (i = 1;i < k;i++){
			leftxor(section_buf[j],section_buf[i],(j-k)*i*sizeof(DT),sz);
		}
	}
}


/*
result: this is a 1-dimemsion array, whith (k+m) elements.
section_buf: a 2-dimemsion array, with (k+m) rows, sz columns.
	all the siginifactive data is stored in section_buf.
if result[i] is not 0, the section_buf[i] is siginifactive.
otherwise, result[i] is 0, then section_buf[i] is lost and will make a repair.
 */
int decode(int k, int m, BYTE **section_buf, int* result, int sz){
	sz-=(k-1)*(m-1)*sizeof(DT);	if(sz<=0) return -2;
	int i,j;


	int goodCode=0;
	int numRepairO=0;
	for(i=0;i<k;i++) if(result[i]==0) numRepairO++;

	if(numRepairO>m) {return -1;}
	if(numRepairO==0) {return 0;}


	int cexist[MAX_K]={0},olost[MAX_K]={0};
	int mc[MAX_K][MAX_K]={0};
	int n=k+m;
//	for(i=0;i<k+m;i++) if(result[i]==0) memset(section_buf[i],0,30);

	DT * coding[MAX_K]={0};
	DT * repair[MAX_K]={0};

	//check the result[]
	for (i=0;i<n-k;i++){	
		if (result[i+k]!=0){
			cexist[goodCode++]=i;
		}
	}
	numRepairO=0;
	for (i=0;i<k;i++){		
		if (result[i]==0){
			repair[numRepairO]=(DT*)section_buf[i];
			olost[numRepairO++]=i;
		}
	}

	for (i=0;i<numRepairO;i++){
		for(j=0;j<numRepairO;++j){
			mc[i][j]=cexist[i]*olost[j];
		}
	}
	//remove all the existing data blocks from the parity/coding blocks.
	for (j=0;j<numRepairO;j++){
		int cline=cexist[j];
		coding[j]=(DT*)section_buf[cexist[j]+k];
		for (i=0;i<k;i++)
		{
			if(result[i]!=0)
				leftxor((BYTE*)coding[j],(BYTE*)section_buf[i],cline*i*sizeof(DT),sz);
		}
	}

	ker_decode(numRepairO,coding,repair,mc,sz/sizeof(DT)+(k-1)*(m-1),sz/sizeof(DT));//


	return 0;
}