#ifndef PREDICTION_TABLE_H   // Include guard to prevent multiple inclusions
#define PREDICTION_TABLE_H

#include <inttypes.h>
#include "table_element.h"

class PredictionTable
{
    public:
        TableElement** PredictionTableArray;
        uint32_t GlobalBranchHistoryRegister;
        uint32_t PredictionElementsCount;
        int MisPredictions = 0;
        uint32_t NValue;

        PredictionTable(uint32_t mValue, uint32_t nValue)
        {
            NValue = nValue;
            GlobalBranchHistoryRegister = 0 << nValue;
            if (mValue < 0 || mValue > 31)
            {
                return;
            }

            PredictionElementsCount = 1u << mValue;
            PredictionTableArray = new TableElement*[PredictionElementsCount];
            for (uint32_t i = 0; i < PredictionElementsCount; ++i) 
            {
                PredictionTableArray[i] = new TableElement(2);
            }
        };

        void UpdateCounter(uint32_t index, bool isReallyTaken)
        {
            if (PredictionTableArray[index]->IsTaken() != isReallyTaken)
            {
                MisPredictions++;
            }

            if (isReallyTaken)
            {
                PredictionTableArray[index]->IncrementPredictedValue();
            }
            else
            {
                PredictionTableArray[index]->DecrementPredictedValue();
            }
        }

        void UpdateGlobalRegister(bool isReallyTaken)
        {
            if (NValue > 0)
            {
                GlobalBranchHistoryRegister = GlobalBranchHistoryRegister >> 1;
                uint32_t msbFlag = static_cast<uint32_t>(isReallyTaken) << (NValue - 1);
                GlobalBranchHistoryRegister = GlobalBranchHistoryRegister | msbFlag;
            }
        }
};

#endif