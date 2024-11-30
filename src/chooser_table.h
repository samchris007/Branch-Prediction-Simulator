
#ifndef CHOOSER_TABLE_H   // Include guard to prevent multiple inclusions
#define CHOOSER_TABLE_H

#include <inttypes.h>
#include "table_element.h"

class ChooserTable
{
    public:
        TableElement** ChooserTableArray;
        uint32_t PredictionElementsCount;
        
        ChooserTable(int kValue)
        {
            if (kValue < 0 || kValue > 31)
            {
                return;
            }
            PredictionElementsCount = 1 << kValue;
            ChooserTableArray = new TableElement*[PredictionElementsCount];
            for (uint32_t i = 0; i < PredictionElementsCount; ++i) 
            {
                ChooserTableArray[i] = new TableElement(1);
            }
        }

        void UpdateCounter(uint32_t index, bool isGshareCorrect, bool isBiModalCorrect)
        {
            if (!isGshareCorrect && isBiModalCorrect)
            {
                ChooserTableArray[index]->DecrementPredictedValue();
            }
            else if (isGshareCorrect && !isBiModalCorrect)
            {
                ChooserTableArray[index]->IncrementPredictedValue();
            }
        }
};

#endif