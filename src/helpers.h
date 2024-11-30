#ifndef HELPERS_H   // Include guard to prevent multiple inclusions
#define HELPERS_H

#include <inttypes.h>

class Helpers
{
    public:
        static void GetIndexBits(uint32_t address, int mValue, uint32_t& indexBits, uint32_t branchHistoryReg = 0, int nValue = 0) 
        {
            address = address >> 2;
            uint32_t mask = (1 << mValue) - 1;
            uint32_t initialIndexBits = address & mask;
            if (nValue == 0)
            {
                indexBits = initialIndexBits;
                return;
            }
            uint32_t mAndnMask = (1 << (mValue - nValue)) - 1;
            uint32_t lowerBits = initialIndexBits & mAndnMask;
            uint32_t upperBits = initialIndexBits >> (mValue - nValue);
            mask = (1u << nValue) - 1;
            upperBits = upperBits & mask;
            upperBits = branchHistoryReg ^ upperBits;
            indexBits = (upperBits << (mValue - nValue)) | lowerBits;
        }

        static void GetIndexBitsForChooserTable(uint32_t address, int kValue, uint32_t& indexBits) 
        {
            address = address >> 2;
            uint32_t mask = (1 << kValue) - 1;
            indexBits = address & mask;
        }
};

#endif