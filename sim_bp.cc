#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sim_bp.h"
#include "src/table_element.h"
#include "src/chooser_table.h"
#include "src/prediction_table.h"
#include "src/helpers.h"

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
    argv[0] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Branch Prediction\\Branch-Predictor\\sim_bp.cc");
    argv[1] = strdup("hybrid");
    argv[2] = strdup("5");
    argv[3] = strdup("10");
    argv[4] = strdup("7");
    argv[5] = strdup("5");
    argv[6] = strdup("C:\\Users\\samch\\OneDrive\\Documents\\NCSU\\563\\Branch Prediction\\Branch-Predictor\\benchmarks\\jpeg_trace.txt");
    argc = 7;

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
