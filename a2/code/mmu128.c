/*
* Haolun Huang
* EECS 3221 project3
* York University
* id: allen95
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE 16

struct TlbEntry 
{
    int pageNumber;
	int frameNumber;
	int valid;
	//int updateTime;
};

struct PageTableEntry 
{
	int pageNumber;
	int frameNumber;
	int valid;  //0 is false 1 is true
	int updateTime;
};
//prototypes
int searchTLB(int pageNumber);
int searchPageTable(int pageNumber);
int readBacking(int virtualAddress, signed char frameData[]);
int updateMEM(signed char frameData[], int pageNumber);
int replaceMEM(signed char frameData[], int temppageNumber, int pageNumber);
int findLeastRecent();


//physical memmory of 128
char MEM[128][256];
struct TlbEntry TLB[16];
struct PageTableEntry PageTable[256];
int tlbIndex = 0;
int freeFrame = 0;//count avaliable physical memmory space
int newFrameNumber = 0;
int timmer = 0; //global timmer to find which is the least reacent use

int main(int argc, char *argv[])
{	
	//check input format
	if(argc != 4){
		printf("should be <executable><bin file><address list>");
	}
	const char *backing = argv[2];
	const char *inputlist = argv[3];
	const char *outputfile = "output128.csv";
	//reading file
    
	FILE *input = fopen(inputlist, "r");
	FILE *output = fopen(outputfile, "w");
    char line[LINE];
	int logicalAddress;
	int pageNumber, pageOffset, frameNumber;
	int data;
	
	int count = 0;
	int TLBhitcount = 0;
	int pagefaultcount = 0;
   
    
    while (fgets(line,LINE,input) != NULL) {

		//increase total count
		count++;
        logicalAddress = atoi(line);
		pageNumber = (logicalAddress >> 8) & 0xFF;
		pageOffset = logicalAddress & 0xFF;
		frameNumber = searchTLB(pageNumber);
		if (frameNumber >= 0) {//if exist in TLB
            //update reletive page entry time
            PageTable[pageNumber].updateTime = timmer;
			data = MEM[frameNumber][pageOffset];
			TLBhitcount++;
		}
		else {//not in TLB
			frameNumber = searchPageTable(pageNumber);//search from pageTable
			if (frameNumber >= 0) {//if exist in pageTable
                //update relative page time
                PageTable[pageNumber].updateTime = timmer;
				data = MEM[frameNumber][pageOffset];			
			}
			else {//if also not in pageTable, search from backing store
				pagefaultcount++;
				signed char FrameData[256];
                int virtualAddress = logicalAddress - pageOffset;
                if(freeFrame < 128){//when there is space in memory
                    readBacking(virtualAddress, FrameData);
				    updateMEM(FrameData, pageNumber);
				    frameNumber = searchPageTable(pageNumber);
				    if (frameNumber >= 0) {
					    data = MEM[frameNumber][pageOffset];			
				    }
                }
                else{//physical memory size is full need replacement
                    int temppage = -1;
                    readBacking(virtualAddress, FrameData);
                    //find least recent use by checking update time store in table
                    temppage = findLeastRecent();
                    replaceMEM(FrameData, temppage, pageNumber);
                    //reset relative tlb
                    for(int i=0; i<16;i++){
		                if(TLB[i].pageNumber == temppage){
                            TLB[i].valid = 0;	
		                }
	                }
                    frameNumber = searchPageTable(pageNumber);
				    if (frameNumber >= 0) {
					    data = MEM[frameNumber][pageOffset];			
				    }
                    /*data debug
                    if(logicalAddress == 9772){
                    int physical = ((unsigned int)frameNumber<< 8) | pageOffset;
                    printf("input 39: %d, %d, %d\n", logicalAddress,frameNumber, physical);
                    }*/
                }
				
			}
            //update TLB
	        TLB[tlbIndex].pageNumber = pageNumber;
	        TLB[tlbIndex].frameNumber = searchPageTable(pageNumber);
            TLB[tlbIndex].valid = 1;
	        tlbIndex = (tlbIndex +1) % 16;//regarding FIFO rule
            
		}
		//translate logical to physical address
		int physical = ((unsigned int)frameNumber<< 8) | pageOffset;
		fprintf(output,"%d,%d,%d\n", logicalAddress, physical, data);
		/*data debug
        if(logicalAddress == 2035){
            int test = (logicalAddress >> 8) & 0xFF;
            printf("input 39: %d, %d, %d\n", logicalAddress,freeFrame, data);
        }*/

        //increase timmer
        timmer++;
        
    }
	fprintf(output,"Page Faults Rate, %.2f%%,\n", (pagefaultcount / (count * 1.0))*100);
	fprintf(output,"TLB Hits Rate, %.2f%%,", (TLBhitcount / (count * 1.0))* 100);
	//close file
    fclose(input);
	fclose(output);

    return 0;
}

int searchTLB(int pageNumber) //when miss, return -1;when hit, return frameNumber
{
	for(int i=0; i<16;i++){
		if(TLB[i].pageNumber == pageNumber){
            if(TLB[i].valid ==1){
                return TLB[i].frameNumber;
            }	
		}
	}
	return -1;
}

int searchPageTable(int pageNumber) //when miss, return -1;when hit, return frameNumber
{
	if(PageTable[pageNumber].pageNumber == pageNumber){
        if(PageTable[pageNumber].valid == 1){
            return PageTable[pageNumber].frameNumber;
        }
        
    }
	return -1;
}

int readBacking(int virtualAddress, signed char frameData[])
{
   	FILE *bin = fopen("BACKING_STORE.bin","rb");
	//check if read secessfully
	if(bin == NULL){
		printf("read bin file failed");
		exit(0);
	}
   	fseek(bin, virtualAddress, SEEK_SET );
   	fread(frameData, 256, 1, bin);
   	fclose(bin);
}

int updateMEM(signed char frameData[], int pageNumber)
{
	//update MEM
	for(int i =0; i<256;i++){
		MEM[freeFrame][i] = frameData[i];
		newFrameNumber = freeFrame;
	}
	//update pageTable
	PageTable[pageNumber].pageNumber = pageNumber;
	PageTable[pageNumber].frameNumber = newFrameNumber;
	PageTable[pageNumber].valid = 1;
    PageTable[pageNumber].updateTime = timmer;//update pagenumber access time
	freeFrame++;
}

int replaceMEM(signed char frameData[], int temppageNumber, int pageNumber)
{   
    int tempFrame = searchPageTable(temppageNumber); //frame to be replaced in memmory
	//update MEM
	for(int i =0; i<256;i++){
		MEM[tempFrame][i] = frameData[i];
	}
	//update pageTable
	PageTable[pageNumber].pageNumber = pageNumber;
	PageTable[pageNumber].frameNumber = tempFrame;
	PageTable[pageNumber].valid = 1;
    PageTable[pageNumber].updateTime = timmer;//update pagenumber access time
    //reset the original pagenumber since replaced
    PageTable[temppageNumber].pageNumber = -1;
    PageTable[temppageNumber].frameNumber = -1;
    PageTable[temppageNumber].valid = 0;
    PageTable[temppageNumber].updateTime = -1;
	freeFrame++;
}


int findLeastRecent(){
    int time = 9999999;//tmep variable to compare with timmer, largest means the latest access
    int page = -1;
    for(int i =0; i<256; i++){
        if(time > PageTable[i].updateTime && time >= 0){
            if(PageTable[i].valid == 1){
                time = PageTable[i].updateTime;
                page = PageTable[i].pageNumber;
            } 
        }
    }
    return page;
}