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
    int valid; //0 is false 1 is true
               //int updateTime;
};
//prototypes
int searchTLB(int pageNumber);
int searchPageTable(int pageNumber);
int readBacking(int virtualAddress, signed char frameData[]);
int updateMEM(signed char frameData[], int pageNumber);

//physical memmory of 256
char MEM[256][256];
struct TlbEntry TLB[16];
struct PageTableEntry PageTable[256];
int tlbIndex = 0;
int freeFrame = 0;
int newFrameNumber = 0;

int main(int argc, char *argv[])
{
    //check input format
    if (argc != 4)
    {
        printf("should be <executable><bin file><address list>");
    }
    const char *backing = argv[2];
    const char *inputlist = argv[3];
    const char *outputfile = "output256.csv";
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

    while (fgets(line, LINE, input) != NULL)
    {

        //increase total count
        count++;
        logicalAddress = atoi(line);
        pageNumber = (logicalAddress >> 8) & 0xFF;
        pageOffset = logicalAddress & 0xFF;
        frameNumber = searchTLB(pageNumber);

        //if exist in TLB
        if (frameNumber >= 0)
        {
            data = MEM[frameNumber][pageOffset];
            TLBhitcount++;
        }
        else
        {                                              //not in TLB
            frameNumber = searchPageTable(pageNumber); //search from pageTable
            if (frameNumber >= 0)
            { //if exist in pageTable
                data = MEM[frameNumber][pageOffset];
            }
            else
            { //if also not in pageTable, search from backing store
                pagefaultcount++;
                signed char FrameData[256];
                int virtualAddress = logicalAddress - pageOffset;
                readBacking(virtualAddress, FrameData);
                updateMEM(FrameData, pageNumber);
                frameNumber = searchPageTable(pageNumber);
                if (frameNumber >= 0)
                {
                    data = MEM[frameNumber][pageOffset];
                }
            }
            //update TLB
            TLB[tlbIndex].pageNumber = pageNumber;
            TLB[tlbIndex].frameNumber = searchPageTable(pageNumber);
            TLB[tlbIndex].valid = 1;
            tlbIndex = (tlbIndex + 1) % 16; //regarding FIFO rule
        }
        //translate logical to physical address
        int physical = ((unsigned int)frameNumber << 8) | pageOffset;
        //int result = *((char*)MEM + physical);
        fprintf(output, "%d,%d,%d\n", logicalAddress, physical, data);
        /*data debug
        if(logicalAddress == 39){
            int test = (logicalAddress >> 8) & 0xFF;
            printf("input 39: %d, %d, %d\n", logicalAddress,PageTable[pageNumber].frameNumber, data);
        }
        */
    }
    fprintf(output, "Page Faults Rate, %.2f%%,\n", (pagefaultcount / (count * 1.0)) * 100);
    fprintf(output, "TLB Hits Rate, %.2f%%,", (TLBhitcount / (count * 1.0)) * 100);
    //close file
    fclose(input);
    fclose(output);

    return 0;
}

int searchTLB(int pageNumber) //when miss, return -1;when hit, return frameNumber
{
    for (int i = 0; i < 16; i++)
    {
        if (TLB[i].pageNumber == pageNumber)
        {
            if (TLB[i].valid == 1)
            {
                return TLB[i].frameNumber;
            }
        }
    }
    return -1;
}

int searchPageTable(int pageNumber) //when miss, return -1;when hit, return frameNumber
{
    if (PageTable[pageNumber].pageNumber == pageNumber)
    {
        if (PageTable[pageNumber].valid == 1)
        {
            return PageTable[pageNumber].frameNumber;
        }
    }
    return -1;
}

int readBacking(int virtualAddress, signed char frameData[])
{
    FILE *bin = fopen("BACKING_STORE.bin", "rb");
    //check if read secessfully
    if (bin == NULL)
    {
        printf("read bin file failed");
        exit(0);
    }
    fseek(bin, virtualAddress, SEEK_SET);
    fread(frameData, 256, 1, bin);
    fclose(bin);
}

int updateMEM(signed char frameData[], int pageNumber)
{
    //update MEM
    for (int i = 0; i < 256; i++)
    {
        MEM[freeFrame][i] = frameData[i];
        newFrameNumber = freeFrame;
    }
    //update pageTable
    PageTable[pageNumber].pageNumber = pageNumber;
    PageTable[pageNumber].frameNumber = newFrameNumber;
    PageTable[pageNumber].valid = 1;
    freeFrame++;
}