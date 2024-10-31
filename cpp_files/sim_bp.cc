#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <cmath>
#include "sim_bp.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/



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

        // void IncrementMisPredictionsIfPredictionIsCorrect(uint32_t index, bool isReallyTaken)
        // {
        //     if (PredictionTableArray[index]->IsTaken() != isReallyTaken)
        //     {
        //         MisPredictions++;
        //     }
            
        //     if (NValue > 0)
        //     {
        //         GlobalBranchHistoryRegister = GlobalBranchHistoryRegister >> 1;
        //         uint32_t msbFlag = static_cast<uint32_t>(isReallyTaken) << (NValue - 1);
        //         GlobalBranchHistoryRegister = GlobalBranchHistoryRegister | msbFlag;
        //     }
        // }

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

void ShowChooserContents(ChooserTable chooserTable)
{
    printf("FINAL CHOOSER CONTENTS\n");
    for (int i = 0; i < chooserTable.PredictionElementsCount; i++)
    {
        printf("%d %d\n", i, chooserTable.ChooserTableArray[i]->PredictedValue);
    }
}

void ShowGshareContents(PredictionTable predictionTable)
{
    printf("FINAL GSHARE CONTENTS\n");
    for (int i = 0; i < predictionTable.PredictionElementsCount; i++)
    {
        printf("%d %d\n", i, predictionTable.PredictionTableArray[i]->PredictedValue);
    }
}

void ShowBiModal(PredictionTable predictionTable)
{
    printf("FINAL BIMODAL CONTENTS\n");
    for (int i = 0; i < predictionTable.PredictionElementsCount; i++)
    {
        printf("%d %d\n", i, predictionTable.PredictionTableArray[i]->PredictedValue);
    }
}


int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    // argv[0] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Branch Prediction\\Branch-Predictor\\cpp_files\\sim_bp.cc");
    // argv[1] = strdup("hybrid");
    // argv[2] = strdup("5");
    // argv[3] = strdup("10");
    // argv[4] = strdup("7");
    // argv[5] = strdup("5");
    // argv[6] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Branch Prediction\\Branch-Predictor\\jpeg_trace.txt");
    // argc = 7;

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    
    char str[2];
    PredictionTable predictionTableForBiModal = PredictionTable(params.M2, 0);
    PredictionTable predictionTableForGShare = PredictionTable(params.M1, params.N);
    ChooserTable chooserTable = ChooserTable(params.K);
    uint32_t predictions = 0;
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        outcome = str[0];
        bool isReallyTaken = outcome == 't';
        predictions++;
        if(strcmp(params.bp_name, "bimodal") == 0)
        {
            uint32_t indexBits;
            Helpers::GetIndexBits(addr, params.M2, indexBits);
            predictionTableForBiModal.UpdateCounter(indexBits, isReallyTaken);
        }
        else if(strcmp(params.bp_name, "gshare") == 0) 
        {
            uint32_t indexBits;
            Helpers::GetIndexBits(addr, params.M1, indexBits,  predictionTableForGShare.GlobalBranchHistoryRegister, params.N);
            predictionTableForGShare.UpdateCounter(indexBits, isReallyTaken);
            predictionTableForGShare.UpdateGlobalRegister(isReallyTaken);
        }
        else
        {
            uint32_t indexBitsForBiModal, indexBitsForGShare, indexBitsForChooser;
            Helpers::GetIndexBits(addr, params.M2, indexBitsForBiModal);
            Helpers::GetIndexBits(addr, params.M1, indexBitsForGShare,  predictionTableForGShare.GlobalBranchHistoryRegister, params.N);
            Helpers::GetIndexBitsForChooserTable(addr, params.K, indexBitsForChooser);
            if (chooserTable.ChooserTableArray[indexBitsForChooser]->UseGSharePredictor())
            {
                chooserTable.UpdateCounter(indexBitsForChooser, 
                                    predictionTableForGShare.PredictionTableArray[indexBitsForGShare]->IsTaken() == isReallyTaken,
                                    predictionTableForBiModal.PredictionTableArray[indexBitsForBiModal]->IsTaken() == isReallyTaken);
                predictionTableForGShare.UpdateCounter(indexBitsForGShare, isReallyTaken);
            }
            else
            {
                chooserTable.UpdateCounter(indexBitsForChooser, 
                                    predictionTableForGShare.PredictionTableArray[indexBitsForGShare]->IsTaken() == isReallyTaken,
                                    predictionTableForBiModal.PredictionTableArray[indexBitsForBiModal]->IsTaken() == isReallyTaken);
                predictionTableForBiModal.UpdateCounter(indexBitsForBiModal, isReallyTaken);
            }
            predictionTableForGShare.UpdateGlobalRegister(isReallyTaken);
        }
    }
    printf("OUTPUT\n");
    printf("number of predictions: %d\n", predictions);
    double percentage;
    if(strcmp(params.bp_name, "bimodal") == 0)
    {
        printf("number of mispredictions: %6d\n", (int)predictionTableForBiModal.MisPredictions); 
        percentage = ((double)predictionTableForBiModal.MisPredictions / (double)predictions) * 100;
        printf("misprediction rate: %11.2f%\n", percentage);
        ShowBiModal(predictionTableForBiModal);
    }
    else if(strcmp(params.bp_name, "gshare") == 0) {
        printf("number of mispredictions: %6d\n", (int)predictionTableForGShare.MisPredictions);
        percentage = ((double)predictionTableForGShare.MisPredictions / (double)predictions) * 100;
        printf("misprediction rate: %11.2f%\n", percentage);
        ShowGshareContents(predictionTableForGShare);
    }
    else if(strcmp(params.bp_name, "hybrid") == 0) {
        printf("number of mispredictions: %6d\n", (int)predictionTableForBiModal.MisPredictions + (int)predictionTableForGShare.MisPredictions);
        percentage = ((double)(predictionTableForBiModal.MisPredictions + predictionTableForGShare.MisPredictions) / (double)predictions)  * 100;
        printf("misprediction rate: %11.2f%\n", percentage);
        ShowChooserContents(chooserTable);
        ShowGshareContents(predictionTableForGShare);
        ShowBiModal(predictionTableForBiModal);
    }

    return 0;
}
