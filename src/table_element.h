#ifndef TABLE_ELEMENT_H   // Include guard to prevent multiple inclusions
#define TABLE_ELEMENT_H

class TableElement 
{  
    public:
        int PredictedValue;

        TableElement(int num)
        {
            PredictedValue = num;
        }

        void IncrementPredictedValue() 
        {
            if (PredictedValue == 3)
            {
                return;
            }
            PredictedValue++;
        }

        void DecrementPredictedValue() 
        {
            if (PredictedValue == 0) 
            {
                return;
            }
            PredictedValue--;
        }

        bool IsTaken()
        {
            return PredictedValue > 1;
        }

        bool UseGSharePredictor()
        {
            return PredictedValue > 1;
        }
};

#endif