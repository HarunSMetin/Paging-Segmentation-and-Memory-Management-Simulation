/*
Harun Serkan Metin  

COMPILE : 
	gcc main.c -o main
    
RUN :
	./main	<ProgramName> <Ref.txt>
	#OR
	./main	<Program Name> 
	#In this usage program automatically create 100 lines ref.txt and it will use it 

EXAMPLE USAGE:
	./main a.out ref.txt
	#OR
	./main a.out
*/
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#define ceil(a,b) ((a / b) + ((a % b) != 0) )

typedef struct entry
{
    int segNum;
    int pageNum;
}entry;
void delay(int milli_seconds)
{
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds*1000)
        ;
}

int add2TLB(entry *TLB,entry *E){
    int i =0;
    for(i =0;i<50;i++){
        if (TLB[i].pageNum==-1 && TLB[i].segNum==-1){
            TLB[i]=*E;
            return 1;
        }
    }
    for(i =0;i<49;i++){
        TLB[i]=TLB[i+1];
    }
    TLB[49]=*E;
    return 1;

}
int add2Mem(entry* Mem,int sizeMem,long* dates,entry *E){
    int i =0;
    for(i =0;i<sizeMem;i++){
        if (Mem[i].pageNum==-1 && Mem[i].segNum==-1){
            Mem[i]=*E;
            time_t now = time(NULL);
            long double curTime=(long double)now;
            dates[i] = curTime; 
            return 1;
        }
    }
    int min =0;
    for(i =0;i<sizeMem;i++){
        if(dates[min] >= dates[i])min =i;
    }
    Mem[min]=*E;
    time_t now = time(NULL);
    long double curTime=(long double)now;
    dates[min] = curTime; 
    return 1;
}

void refMake(int seg,int page0,int page1,int page2,int FaultMode ){
    srand(time(NULL));  
    int i;
    FILE *fptr;
    fptr=fopen("ref.txt","w"); 
    for(i=0;i<100;i++){
        int segment = rand()%seg;  
        int page =0; 
        switch (segment)
        {
        case 0:
            page=rand()%(page0+FaultMode);    
            break;
        case 1:
            page=rand()%(page1+FaultMode);    
            break;
        case 2:
            page=rand()%(page2+FaultMode);    
            break;
        } 
          
        fprintf(fptr,"%d %d\n",segment,page);
        
    }
    fclose(fptr);
}

int main(int argc, char const *argv[])
{

    char fileName[20];
    char segFileName[20];

    if(argc>1){
        if(argc==3) strcpy(fileName,argv[2]);
    strcpy(segFileName,argv[1]);
    }
    char *seg = (char*)malloc(100 * sizeof(char));
    sprintf(seg, "%s %s%s", "size",segFileName," > segmentInfo.txt");
    system(seg);

    int arr[3];
    int counter=0;        
    
    FILE *segfp = fopen("segmentInfo.txt","r");
 
    char buf[10];
    while( fscanf(segfp, "%s", buf) != EOF )
    {
       if(counter>5 && counter<9){
            if(atoi(buf)%1024==0)
                arr[counter-6]=(int)(atoi(buf)/1024); 
            else
                arr[counter-6]=(int)(atoi(buf)/1024)+1; 
        }
        counter++;  
    }

    if(argc==2){
        printf("\nThere is no Referance file. I am creating\n");
        strcpy(fileName,"ref.txt");
        //refMake(3,arr[0],arr[1],arr[2],0);//Page Fault vermeyecek sorguların olduğu bir ref.txt yaratıryor
        refMake(3,arr[0],arr[1],arr[2],1);//Hatasız ve Hatalı sorguların olduğu bir ref.txt yaratıryor
    }
    
    printf("Pages Counts:\nSeg0: %d\nSeg1: %d\nSeg2: %d\n",arr[0],arr[1],arr[2]);
    int totalPage=arr[0]+arr[1]+arr[2];
    printf("Total Page Counts:%d\n",totalPage);
  ///////////////////////////////////////////////////////////////////////////////////////
  //CREATING TLB
   
    entry *TLB=(entry*)malloc(50*sizeof(entry));
    int i;
    for(i=0;i<50;i++){
        TLB[i].pageNum=-1;
        TLB[i].segNum=-1;
    }
    /////////////////////////////////////////////////////////////////
    //CREATING Virtual Memory
    entry *VMem = (entry*)malloc(totalPage*sizeof(entry));
    for(i=0;i<arr[0];i++)
    {
        VMem[i].pageNum=i;
        VMem[i].segNum=0;
    }
    for(i=0;i<arr[1];i++)
    {
        VMem[i].pageNum=i;
        VMem[i].segNum=1;
    }
    for(i=0;i<arr[2];i++)
    {
        VMem[i].pageNum=i;
        VMem[i].segNum=2;
    }

    ////////////////////////////////////////////////////////////////
    //CREATING Phisical Memory
    int sizeMem=ceil(arr[0],2)+ceil(arr[1],2)+ceil(arr[2],2);
    entry *memory=(entry*)malloc(sizeMem*sizeof(entry));
    long dates[sizeMem]; //dates for LRU
    for(i=0;i<sizeMem;i++) dates[i]=0;

    for(i=0;i<sizeMem;i++)
    {
        TLB[i].pageNum=-1;
        TLB[i].segNum=-1;
        if(i<3){
            memory[i].pageNum=0;
            memory[i].segNum=i;
            time_t now = time(NULL);
            long double curTime=(long double)now;
            dates[i] = curTime; 

        }
    }
 
    ////////////////////////////////////////////////////////////////
    //Okunacak Dosyada Kac Satır Var
    FILE *fpCOUNT = fopen(fileName, "r");
    char c;
    int line=0;
    for (c = getc(fpCOUNT); c != EOF; c = getc(fpCOUNT))if (c == '\n')line++;

    ////////////////////////////////////////////////////////////////
    //ref.txt dosyasını oku SP 2boyutlu arayinin elemanlarına ata
    int SP[line][2];

    FILE *fp = fopen(fileName,"r");
 
    buf[10];
    int flag=0;
        while( fscanf(fp, "%s", buf) != EOF )
        {
            int index = (int)floor(flag/2);
            SP[index][flag%2]=atoi(buf); 
            flag++;
        }
  
    //////////////////////////////////////////////////////////////////
    int j,k,l,TLBmiss=0,VMmiss=0, pageFault=0,totalDelay=0;
    int tlbFoundFlag=0,vmFoundFlag=0;
    for(i=0;i<line;i++){
        entry *E = (entry*)malloc(sizeof(entry));
        E->segNum=SP[i][0];
        E->pageNum=SP[i][1];
        printf("---------------------------\nLooking For Seg: %d, Page: %d\n", E->segNum,E->pageNum);
        printf("TLB:\nSEG\tPAGE\n");
        for(j=0;j<50;j++){
            printf("%d\t%d\n",TLB[j].segNum ,TLB[j].pageNum);
            if(TLB[j].segNum == SP[i][0] && TLB[j].pageNum==SP[i][1]){
                printf("Found in TLB Seg: %d Page: %d\n\n",SP[i][0],SP[i][1]);
                delay(1);
                totalDelay++;
                break;
            }
            else if(j==49){
                printf("NOT Found in TLB Seg: %d Page: %d ---TLB Miss---\n",SP[i][0],SP[i][1]);
                TLBmiss++;
                printf("MEMORY:\nSEG\tPAGE\tDATE\n");
                for(k=0;k<sizeMem;k++){
                    printf("%d\t%d\t%ld\n",memory[k].segNum ,memory[k].pageNum,dates[k]);
                    if(memory[k].segNum == SP[i][0] && memory[k].pageNum==SP[i][1]){
                        printf("Found in Physical Memory Seg: %d Page: %d\n\n",SP[i][0],SP[i][1]);
                        add2TLB(TLB,E);
                        delay(10);
                        totalDelay+=10;
                        break;
                    }
                    else if(k==sizeMem-1){
                        printf("NOT Found in Physical Memory  Seg: %d Page: %d ---Physical Memory Miss---\n\n",SP[i][0],SP[i][1]);
                        VMmiss++;
                        for(l=0;l<totalPage;l++){
                            if(VMem[l].segNum == SP[i][0] && VMem[l].pageNum==SP[i][1]){
                                printf("Found in Virtual Memory Seg: %d Page: %d\n\n",SP[i][0],SP[i][1]);
                                add2TLB(TLB,E);
                                add2Mem(memory,sizeMem,dates,E);
                                break;
                            }
                            else if(l==totalPage-1){
                                printf("NOT Found in Virtual Memory  Seg: %d Page: %d ---Virtual Memory Miss---\n",SP[i][0],SP[i][1]);
                                delay(100);
                                totalDelay+=100;
                                pageFault++;
                            }
                        }
                    }
                }
            }
        }   
    }
    printf("\n\nTotal Query Count: %d\nTLB miss: %d\nVM miss: %d\nPage Fault: %d\n\nTotal Delay: %d\n",line,TLBmiss,VMmiss, pageFault,totalDelay);

   return 0;
}


