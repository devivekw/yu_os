#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "util.c"

#define INPUT_LENGTH 7

// Main Functions
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        return 0;
    }

    // input is -> ./mmu 256 BACKING_STORE.BIN address.txt
    char *backingFile = argv[2];
    FILE *addressInput = fopen(argv[3], "r");
    char inputLine[INPUT_LENGTH];

    // output file
    int TOTAL_FRAMES = atoi(argv[1]);
    bool is256 = TOTAL_FRAMES == 256;
    char outName[13];
    strcpy(outName, "output");
    strcat(outName, argv[1]);
    strcat(outName, ".csv");
    FILE *outFile = fopen(outName, "w");

    // Table Info
    int virtualAddr, localPageNo, localPageOffset, localFrameNo, memData;

    // Counter Info
    int total = 0, HITCounter = 0, FAULTCounter = 0;

    // Traverse Line by Line
    while (fgets(inputLine, INPUT_LENGTH, addressInput) != NULL)
    {
        // set table info
        virtualAddr = atoi(inputLine);
        localPageNo = (atoi(inputLine) >> 8) & 0xFF;
        localPageOffset = (atoi(inputLine)) & 0xFF;

        // Get  FrameNo from TLB
        localFrameNo = getFrameTLB(localPageNo);

        // If exists in TLB
        if (localFrameNo >= 0)
        {
            // update TLB hit counter
            HITCounter += 1;
            updateTimePT(localPageNo, total);
            memData = getMEMData(localFrameNo, localPageOffset, is256);
        }
        // else search in page   table
        else
        {
            localFrameNo = getFramePT(localPageNo);
            // check if exists in PAGE Table
            if (localFrameNo >= 0)
            {
                // Update latest used page time
                updateTimePT(localPageNo, total);
                memData = getMEMData(localFrameNo, localPageOffset, is256);
            }
            // if not in page table, get from BACKING_STORE
            else
            {
                // increment Page Fault counter
                FAULTCounter += 1;

                // Temp backing data
                char BACKING_DATA[256];
                // get data from backing
                readBackingStore(backingFile, BACKING_DATA, virtualAddr - localPageOffset);

                // If frame is availble in Memory
                int availFrameNo = getAvailableFrameNo();

                
                if (availFrameNo < TOTAL_FRAMES)
                {
                    // update PHYSICAL_MEMORY and PAGE_TABLE
                    updateMEMData(BACKING_DATA, is256);
                    updateFramePT(localPageNo);
                    updateTimePT(localPageNo, total);

                   
                }               
                // Else if frame is not available in Memory
                else
                {
                    // Least Recently used Page number
                    int LRUPageNo = getLRUPageNo();
                    int LRUFrameNo = getFramePT(LRUPageNo);

                    // udpate Frame Memory and Page Table in order
                    resetMEMDataLRU(BACKING_DATA, LRUFrameNo, is256);
                    resetFramePTLRU(localPageNo, LRUFrameNo, total, LRUPageNo);

                    // also invalidate in TLB if exists
                    invalidateLRUTLB(LRUPageNo);
                }
            }

            // get updated frame Number
            localFrameNo = getFramePT(localPageNo);
            memData = getMEMData(localFrameNo, localPageOffset, is256);
            
            // update TLB if not found in TLB and Maintain FIFO
            updateTLB(localPageNo, localFrameNo);
        }

        total++;
        int physicalAddr = ((unsigned int)localFrameNo << 8) | localPageOffset;

        fprintf(outFile, "%d,%d,%d\n", virtualAddr, physicalAddr, memData);
        // printf("%d %d %d\n", virtualAddr, physicalAddr, memData);
    }
    
    fprintf(outFile, "Page Faults Rate, %.2f%%,\n", (FAULTCounter/(total * 1.0))* 100);
    fprintf(outFile, "TLB Hits Rate, %.2f%%,", (HITCounter/(total * 1.0))* 100);


    fclose(addressInput);
    fclose(outFile);
    // printf("end total %d \n", total);
    return 0;
}
