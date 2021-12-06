#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

// Structure for storing the TLB Entrys and PageTable Entrys
struct EntryStruct
{
    int pageNo;
    int frameNo;
    unsigned int validBit;
    int usedTime;
};

// 2D array for storing 256 byte information for the BACKING_STORE
char PHYSICAL_MEM256[256][256];
char PHYSICAL_MEM128[128][256];

// Page Table Info
struct EntryStruct PAGE_TABLE[256];
int AvailableFrameNo = 0;

// TLB Ref Information
struct EntryStruct TLB[16];
int TLBCounter = 0;

// TLB
// get frame number if exists in TLB
int getFrameTLB(int pageNo)
{
    for (int j = 0; j < 16; j++)
    {
        if (TLB[j].pageNo == pageNo && TLB[j].validBit != 0)
        {
            return TLB[j].frameNo;
        }
    }
    return -1;
}

void updateTLB(int pageNo, int frameNo) {
    TLB[TLBCounter].pageNo = pageNo;
    TLB[TLBCounter].frameNo = frameNo;
    TLB[TLBCounter].validBit = 1;
    TLBCounter = (TLBCounter + 1) % 16; // update FIFO with size 16
}

void invalidateLRUTLB(int pageNo) {
    for (int j = 0; j < 16; j++) {
        if (TLB[j].pageNo == pageNo) {
            TLB[j].validBit = 0;
        }
    }
}

// MEMORY
// GET Memory Data from PageNo & PageOffset
int getMEMData(int frameNo, int pageOffset, bool is256)
{
    if (is256) {
        return PHYSICAL_MEM256[frameNo][pageOffset];
    } else {
        return PHYSICAL_MEM128[frameNo][pageOffset];
    }
}

void updateMEMData(char BACKING_DATA[], bool is256)
{
    for (int i = 0; i < 256; i++)
    {
        if (is256) {
            PHYSICAL_MEM256[AvailableFrameNo][i] = BACKING_DATA[i];
        } else {
            PHYSICAL_MEM128[AvailableFrameNo][i] = BACKING_DATA[i];
        }
    }
}

void resetMEMDataLRU(char BACKING_DATA[], int frameNo, bool is256)
{
    for (int i = 0; i < 256; i++)
    {
        if (is256) {
            PHYSICAL_MEM256[frameNo][i] = BACKING_DATA[i];
        } else {
            PHYSICAL_MEM128[frameNo][i] = BACKING_DATA[i];
        }
    }
}

void readBackingStore(char *backingFile, char BACKING_DATA[], int addr)
{
    FILE *binInput = fopen(backingFile, "rb");

    if (binInput == NULL)
    {
        exit(0);
    }
    fseek(binInput, addr, SEEK_SET);
    fread(BACKING_DATA, 256, 1, binInput);
    fclose(binInput);
}

// PAGE TABLE -> PT
// check is PageNo matches the page table ref
int getFramePT(int pageNo)
{
    if (PAGE_TABLE[pageNo].pageNo == pageNo && PAGE_TABLE[pageNo].validBit != 0)
    {
        return PAGE_TABLE[pageNo].frameNo;
    }
    else
    {
        return -1;
    }
}

void updateFramePT(int pageNo) {
    PAGE_TABLE[pageNo].pageNo = pageNo;
    PAGE_TABLE[pageNo].frameNo = AvailableFrameNo;
    PAGE_TABLE[pageNo].validBit = 1;
    AvailableFrameNo += 1;
}

void resetFramePTLRU(int pageNo, int frameNo, int time, int invalidPageNo) {
    // Update the pageNumbers Page table
    PAGE_TABLE[pageNo].pageNo = pageNo;
    PAGE_TABLE[pageNo].frameNo = frameNo;
    PAGE_TABLE[pageNo].validBit = 1;
    PAGE_TABLE[pageNo].usedTime = time;
    
    // Invalidate the LRU page Number
    PAGE_TABLE[invalidPageNo].pageNo = -99;
    PAGE_TABLE[invalidPageNo].frameNo = -99;
    PAGE_TABLE[invalidPageNo].validBit = 0;
    PAGE_TABLE[invalidPageNo].usedTime = -99;

    AvailableFrameNo += 1;

}

void updateTimePT(int pageNo, int time) {
    PAGE_TABLE[pageNo].usedTime = time;
}

int getAvailableFrameNo() {
    return AvailableFrameNo;
}

int getLRUPageNo() {
    int pageNo = -1, leastTime = INT_MAX;

    for (int j = 0; j < 256; j++)
    {
        if (leastTime > PAGE_TABLE[j].usedTime && leastTime >= 0 && PAGE_TABLE[j].validBit != 0) {
            leastTime = PAGE_TABLE[j].usedTime;
            pageNo = PAGE_TABLE[j].pageNo;
        }
    }

    return pageNo;
}