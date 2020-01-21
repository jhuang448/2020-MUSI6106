
#include <iostream>
#include <ctime>
#include <cmath>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// util functions
void    showClInfo ();
static void show_usage(char *argv);

void genSin(float*& sinBuffer, float fSampleRateInHz, float inputFreq, float inputInSecond, float fAmp = 100);
void   testBlockUnit(float** ppfSinBuffer, float**& ppfAudioDataOutBuffer, int iNumChannels, int blockSize, int iNumBlock, CCombFilterIf::CombFilterType_t combFilterType);

// test functions
int    testFIR();
int    testIIR();
int    testBlock();
int    testZeros();
int    testZeroDelay();

// constant variable PI (high preicsion needed)
const double PI  =3.141592653589793238463;

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath,
                            sFilterType;
    
    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFileInput = 0, *phAudioFileOutput = 0;

    CAudioFileIf::FileSpec_t stFileSpec;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType;
    
    float                   delayInSecond = 0, gain = 0;
    float                   fMaxDelayLengthInS = 1;

    //showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 11) {
        show_usage(argv[0]);
        
        cout << endl << "Run tests..." << endl;
        
        // test 1
        testFIR();

        // test 2
        testIIR();
        
        // test 3
        testBlock();

        // test 4
        testZeros();
        
        // test 5
        testZeroDelay();
        
        return 0;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (i + 1 == argc){
            cout << "Invalid input arguments." << endl;
            show_usage(argv[0]);
            return -1;
        }
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-i") || (arg == "--input")) {
            sInputFilePath = argv[++i];
        } else if ((arg == "-o") || (arg == "--output")) {
            sOutputFilePath = argv[++i];
        } else if ((arg == "-t") || (arg == "--filter_type")) {
            sFilterType = argv[++i];
            if (sFilterType != "FIR" && sFilterType != "IIR"){
                cout << "Invalid filter type!" << endl;
                show_usage(argv[0]);
                return -1;
            }
        } else if ((arg == "-g") || (arg == "--gain")) {
            gain = std::stof(argv[++i]);
        } else if ((arg == "-d") || (arg == "--delay")) {
            delayInSecond = std::stof(argv[++i]);
        }
        else{
            cout << "Invalid input arguments." << endl;
            show_usage(argv[0]);
            return -1;
        }
    }

//    sInputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep.wav"; //argv[1];
//    sOutputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep_fir.wav"; //argv[2];
//    filterType = "FIR"; //argv[3];
//    delayInSecond = 0.05;
//    gain = 0.1;
    
    if (sFilterType == "FIR") //FIR
        combFilterType = CCombFilterIf::kCombFIR;
    else // IIR
        combFilterType = CCombFilterIf::kCombIIR;

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFileInput);
    phAudioFileInput->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFileInput->isOpen())
    {
        cout << "Input wave file open error!";
        return -1;
    }
    phAudioFileInput->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output wave file
    CAudioFileIf::create(phAudioFileOutput);
    phAudioFileOutput->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phAudioFileOutput->isOpen())
    {
        cout << "Output wave file open error!";
        return -1;
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // instantiate CCombFilterIf
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, gain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, delayInSecond);
    phCombFilter->printStatus();
    
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

    time = clock();
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFileInput->isEof()) //apply filter here!
    {
        long long iNumFrames = kBlockSize;
        phAudioFileInput->readData(ppfAudioData, iNumFrames);

        cout << "\r" << "reading and processing";

        phCombFilter->process(ppfAudioData, ppfAudioData, iNumFrames);
        phAudioFileOutput->writeData(ppfAudioData, iNumFrames);
    }

    cout << "\nreading/processing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFileInput);
    CAudioFileIf::destroy(phAudioFileOutput);
    CCombFilterIf::destroy(phCombFilter);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    return 0;

}

static void show_usage(char *argv)
{
    std::cerr << "Usage: " << argv[0] << " <option(s)> "
    << "Options:\n"
    << "\t-h,--help\t\tShow this help message\n"
    << "\t-i,--input\tInput wave file\n"
    << "\t-o,--output\tOutput wave file\n"
    << "\t-t,--filter_type\tComb Filter Type: FIR or IIR\n"
    << "\t-g,--gain\tgain factor\n"
    << "\t-d,--delay\tdelay in second\n"
    << std::endl;
}

// testFIR: freq matches feedforward
// input: sinusoid, delay: half of the period
// the output should be zero
int testFIR()
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;
    
    float                   **ppfSinBuffer = 0;    // generate sinusoid here
    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;   // FIR
    
    int                     iNumChannels = 1;
    float                   fMaxDelayLengthInS = 1;
    float                   fSampleRateInHz = 44100;
    float                   fInputFreq = 50, fInputInSecond = 3;
    float                   fGain = 1;
    float                   fDelayInSecond = 1 / fInputFreq / 2;

    int                     iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);
    int                     iNumBlocks = floor(fSampleRateInHz * fInputInSecond / kBlockSize);
    
    cout << "----------Start Test 1----------" << endl;

//    cout << "delay in second: " << fDelayInSecond << endl;
//    cout << "delay in sample: " << iDelayInSample << endl;
//    cout << "input frequency: " << fInputFreq << endl;

    ppfAudioData = new float*[1];
    ppfAudioData[0] = new float[kBlockSize];
    ppfAudioDataO = new float*[1];
    ppfAudioDataO[0] = new float[kBlockSize];
    
    // set zero
    for (int i = 0; i < kBlockSize; i++)
        ppfAudioData[0][i] = 0;

    // generate sinusoid
    ppfSinBuffer = new float*[1];
    genSin(ppfSinBuffer[0], fSampleRateInHz, fInputFreq, fInputInSecond);
    
    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * kBlockSize], kBlockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        // check zero
        int st = 0;
        if (i == 0)
            st = iDelayInSample;
        for (int c = 0; c < iNumChannels; c++)
            for (int j = st; j < kBlockSize; j++) {
                if (abs(ppfAudioDataO[c][j]) > 1.0e-3){
                    cout << "Test1 failed." << endl;
                    cout << ppfAudioDataO[c][j] << endl;
                    return -1;
                }
            }
    }

    CCombFilterIf::destroy(phCombFilter);

    delete [] ppfAudioData[0];
    delete [] ppfAudioData;
    delete [] ppfAudioDataO[0];
    delete [] ppfAudioDataO;
    delete [] ppfSinBuffer[0];
    delete [] ppfSinBuffer;
    
    cout << "Test1 passed." << endl;
    
    return 0;
}

// testIIR: freq matches feedback
// input: sinusoid, delay: period
// output: increasing amplitude
// input: sinusoid, delay: half of period
// output: decreasing amplitude
int testIIR()
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;
    
    float                   **ppfSinBuffer = 0;    // generate sinusoid here
    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombIIR;   // IIR
    
    int                     iNumChannels = 1;
    float                   fMaxDelayLengthInS = 1;
    float                   fSampleRateInHz = 44100;
    float                   fInputFreq = 50, fInputInSecond = 3;
    float                   fGain = 0.1;
    float                   fDelayInSecond = 1 / fInputFreq;

    int                     iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);
    int                     iNumBlocks = floor(fSampleRateInHz * fInputInSecond / kBlockSize);
    
    cout << "----------Start Test 2----------" << endl;

   // cout << "delay in second: " << fDelayInSecond << endl;
   // cout << "delay in sample: " << iDelayInSample << endl;
   // cout << "input frequency: " << fInputFreq << endl;

    ppfAudioData = new float*[1];
    ppfAudioData[0] = new float[kBlockSize];
    ppfAudioDataO = new float*[1];
    ppfAudioDataO[0] = new float[kBlockSize];
    
    // set zero
    for (int i = 0; i < kBlockSize; i++)
        ppfAudioData[0][i] = 0;

    // generate sinusoid
    ppfSinBuffer = new float*[1];
    genSin(ppfSinBuffer[0], fSampleRateInHz, fInputFreq, fInputInSecond);
    

    // check: increasing amplitude: delay = period
    fDelayInSecond = 1 / fInputFreq;
    iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);

    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * kBlockSize], kBlockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        // check increasing amplitude
        int st = 0;
        if (i == 0)
            st = iDelayInSample;
        for (int c = 0; c < iNumChannels; c++)
            for (int j = st; j < kBlockSize; j++) {
                // ignore zero
                if (abs(ppfAudioData[c][j]) > 1.0e-10 && abs(ppfAudioDataO[c][j]) < abs(ppfAudioData[c][j])){
                    cout << "Test2: increasing amplitude check failed." << endl;
                    cout << j << ' ' << ppfAudioData[c][j] << " " << ppfAudioDataO[c][j] << endl;
                    return -1;
                }
            }
    }

    CCombFilterIf::destroy(phCombFilter);


    // check: decreasing amplitude: delay = period / 2
    fDelayInSecond = 1 / fInputFreq / 2;
    iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);

    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * kBlockSize], kBlockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        // check decreasing amplitude
        int st = 0;
        if (i == 0)
            st = iDelayInSample;
        for (int c = 0; c < iNumChannels; c++)
            for (int j = st; j < kBlockSize; j++) {
                // ignore zero
                if (abs(ppfAudioData[c][j]) > 1.0e-10 && abs(ppfAudioDataO[c][j]) > abs(ppfAudioData[c][j])){
                    cout << "Test2: decreasing amplitude check failed." << endl;
                    cout << j << ' ' << ppfAudioData[c][j] << " " << ppfAudioDataO[c][j] << endl;
                    return -1;
                }
            }
    }

    CCombFilterIf::destroy(phCombFilter);

    delete [] ppfAudioData[0];
    delete [] ppfAudioData;
    delete [] ppfAudioDataO[0];
    delete [] ppfAudioDataO;
    delete [] ppfSinBuffer[0];
    delete [] ppfSinBuffer;
    
    cout << "Test2 passed." << endl;
    
    return 0;
}

// test_block: varying block
// blocksize: 1024, 500
int testBlock()
{
    int                     kBlockSize1 = 1024, kBlockSize2 = 500;  // 2 different block sizes
    
    float                   **ppfSinBuffer = 0;    // generate sinusoid her

    float                   **ppfOutBuffer1 = 0, **ppfOutBuffer2 = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;   // FIR
    
    int                     iNumChannels = 1;
    
    // params for sinusoid
    float                   fSampleRateInHz = 44100;
    float                   fInputFreq = 50, fInputInSecond = 3;

    int                     iNumBlocks1 = floor(fSampleRateInHz * fInputInSecond / kBlockSize1);
    int                     iNumBlocks2 = floor(fSampleRateInHz * fInputInSecond / kBlockSize2);
    
    cout << "----------Start Test 3----------" << endl;

//    cout << "delay in second: " << fDelayInSecond << endl;
//    cout << "delay in sample: " << iDelayInSample << endl;
//    cout << "input frequency: " << fInputFreq << endl;

        // generate sinusoid
    ppfSinBuffer = new float*[1];
    genSin(ppfSinBuffer[0], fSampleRateInHz, fInputFreq, fInputInSecond);
    int iSinLen = static_cast<int>(fInputInSecond * fSampleRateInHz);
    
    ppfOutBuffer1 = new float*[1];
    ppfOutBuffer1[0] = new float[iSinLen];
    ppfOutBuffer2 = new float*[1];
    ppfOutBuffer2[0] = new float[iSinLen];

    testBlockUnit(ppfSinBuffer, ppfOutBuffer1, iNumChannels, kBlockSize1, iNumBlocks1, combFilterType);
    testBlockUnit(ppfSinBuffer, ppfOutBuffer2, iNumChannels, kBlockSize2, iNumBlocks2, combFilterType);

    for (int c = 0; c < iNumChannels; c++){
        for (int i = 0; i < iNumBlocks1 * kBlockSize1 && i < iNumBlocks2 * kBlockSize2; i++){
            if (ppfOutBuffer1[c][i] != ppfOutBuffer2[c][i]){
                cout << "Test3 failed for FIR." << endl;
                cout << i << " " << ppfOutBuffer1[c][i] << " " << ppfOutBuffer2[c][i] << endl;
                return -1;
            }
        }
    }

    combFilterType = CCombFilterIf::kCombIIR;   // IIR
    testBlockUnit(ppfSinBuffer, ppfOutBuffer1, iNumChannels, kBlockSize1, iNumBlocks1, combFilterType);
    testBlockUnit(ppfSinBuffer, ppfOutBuffer2, iNumChannels, kBlockSize2, iNumBlocks2, combFilterType);

    for (int c = 0; c < iNumChannels; c++){
        for (int i = 0; i < iNumBlocks1 * kBlockSize1 && i < iNumBlocks2 * kBlockSize2; i++){
            if (ppfOutBuffer1[c][i] != ppfOutBuffer2[c][i]){
                cout << "Test3 failed for IIR." << endl;
                cout << i << " " << ppfOutBuffer1[c][i] << " " << ppfOutBuffer2[c][i] << endl;
                return -1;
            }
        }
    }

    delete [] ppfSinBuffer[0];
    delete [] ppfSinBuffer;
    delete [] ppfOutBuffer1[0];
    delete [] ppfOutBuffer1;
    delete [] ppfOutBuffer2[0];
    delete [] ppfOutBuffer2;
    
    cout << "Test3 passed." << endl;
    
    return 0;
}

void testBlockUnit(float** ppfSinBuffer, float**& ppfAudioDataOutBuffer, int iNumChannels, int blockSize, int iNumBlocks, CCombFilterIf::CombFilterType_t combFilterType)
{
    long long               iNumFrames = blockSize;
    
    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    float                   fMaxDelayLengthInS = 1;
    float                   fSampleRateInHz = 44100;
    float                   fGain = 1;
    float                   fDelayInSecond = 0.00227;

    assert(iNumChannels == 1);

    ppfAudioData = new float*[1];
    ppfAudioData[0] = new float[blockSize];
    ppfAudioDataO = new float*[1];
    ppfAudioDataO[0] = new float[blockSize];
    
    // set zero
    for (int i = 0; i < blockSize; i++)
        ppfAudioData[0][i] = 0;

    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * blockSize], blockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        for (int c = 0; c < iNumChannels; c++)
            memcpy(&ppfAudioDataOutBuffer[c][i * blockSize], ppfAudioDataO[c], blockSize * sizeof(float));
    }

    CCombFilterIf::destroy(phCombFilter);

    delete [] ppfAudioData[0];
    delete [] ppfAudioData;
    delete [] ppfAudioDataO[0];
    delete [] ppfAudioDataO;
}

// test_zeros
// test FIR & IIR on one block of zero input
// the output should be zero
int testZeros()
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;

    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;   // FIR, IIR
    
    int                     iNumChannels = 1;
    float                   fMaxDelayLengthInS = 1;
    float                   fSampleRateInHz = 44100;
    float                   fDelayInSecond = 0.00227, fGain = 1;         // freq: 441 Hz
    
    cout << "----------Start Test 4----------" << endl;

    ppfAudioData = new float*[1];
    ppfAudioData[0] = new float[kBlockSize];
    ppfAudioDataO = new float*[1];
    ppfAudioDataO[0] = new float[kBlockSize];
    
    // set zero
    for (int i = 0; i < kBlockSize; i++)
        ppfAudioData[0][i] = 0;

    // test zero input for FIR
    combFilterType = CCombFilterIf::kCombFIR;
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

    // check zero
    for (int i = 0; i < kBlockSize; i++)
        if (abs(ppfAudioDataO[0][i]) > 1.0e-20){
            cout << "Test4: zero input failed for FIR." << endl;
            cout << ppfAudioDataO[0][i] << endl;
            return -1;
        }
    CCombFilterIf::destroy(phCombFilter);
    
    // test zero input for IIR
    combFilterType = CCombFilterIf::kCombIIR;
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);
    
    // check zero
    for (int i = 0; i < kBlockSize; i++)
        if (abs(ppfAudioDataO[0][i]) > 1.0e-20){
            cout << "Test4: zero input failed for IIR." << endl;
            cout << ppfAudioDataO[0][i] << endl;
            return -1;
        }
    CCombFilterIf::destroy(phCombFilter);
    
    delete [] ppfAudioData[0];
    delete [] ppfAudioData;
    delete [] ppfAudioDataO[0];
    delete [] ppfAudioDataO;
    
    cout << "Test4 passed." << endl;
    
    return 0;
}

int testZeroDelay()
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;
    
    float                   **ppfSinBuffer = 0;    // generate sinusoid here
    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;   // FIR
    
    int                     iNumChannels = 1;
    float                   fMaxDelayLengthInS = 1;
    float                   fSampleRateInHz = 44100;
    float                   fInputFreq = 50, fInputInSecond = 3;
    float                   fGain = 1;
    float                   fDelayInSecond = 0;     // zero delay

    int                     iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);
    int                     iNumBlocks = floor(fSampleRateInHz * fInputInSecond / kBlockSize);
    
    cout << "----------Start Test 5----------" << endl;

//    cout << "delay in second: " << fDelayInSecond << endl;
//    cout << "delay in sample: " << iDelayInSample << endl;
//    cout << "input frequency: " << fInputFreq << endl;

    ppfAudioData = new float*[1];
    ppfAudioData[0] = new float[kBlockSize];
    ppfAudioDataO = new float*[1];
    ppfAudioDataO[0] = new float[kBlockSize];
    
    // set zero
    for (int i = 0; i < kBlockSize; i++)
        ppfAudioData[0][i] = 0;

    // generate sinusoid
    ppfSinBuffer = new float*[1];
    genSin(ppfSinBuffer[0], fSampleRateInHz, fInputFreq, fInputInSecond);

    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * kBlockSize], kBlockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        // check zero
        int st = 0;
        if (i == 0)
            st = iDelayInSample;
        for (int c = 0; c < iNumChannels; c++)
            for (int j = st; j < kBlockSize; j++) {
                if (ppfAudioDataO[c][j] != ppfAudioData[c][j]){
                    cout << "Test5 failed for FIR." << endl;
                    cout << ppfAudioDataO[c][j] << endl;
                    return -1;
                }
            }
    }

    CCombFilterIf::destroy(phCombFilter);


    combFilterType = CCombFilterIf::kCombIIR;   // IIR
    // instantiate combfilter
    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(combFilterType, fMaxDelayLengthInS, fSampleRateInHz, iNumChannels);
    phCombFilter->setParam(CCombFilterIf::kParamGain, fGain);
    phCombFilter->setParam(CCombFilterIf::kParamDelay, fDelayInSecond);
    //phCombFilter->printStatus();
    
    for (int i = 0; i < iNumBlocks; i++){
        // read a block from ppfSinBuffer
        for (int c = 0; c < iNumChannels; c++)
            memcpy(ppfAudioData[c], &ppfSinBuffer[c][i * kBlockSize], kBlockSize * sizeof(float));
        
        // apply FIR
        phCombFilter->process(ppfAudioData, ppfAudioDataO, iNumFrames);

        // check zero
        int st = 0;
        if (i == 0)
            st = iDelayInSample;
        for (int c = 0; c < iNumChannels; c++)
            for (int j = st; j < kBlockSize; j++) {
                if (ppfAudioDataO[c][j] != ppfAudioData[c][j]){
                    cout << "Test5 failed for IIR." << endl;
                    cout << ppfAudioDataO[c][j] << endl;
                    return -1;
                }
            }
    }

    CCombFilterIf::destroy(phCombFilter);



    delete [] ppfAudioData[0];
    delete [] ppfAudioData;
    delete [] ppfAudioDataO[0];
    delete [] ppfAudioDataO;
    delete [] ppfSinBuffer[0];
    delete [] ppfSinBuffer;
    
    cout << "Test5 passed." << endl;
    
    return 0;
}

void genSin(float*& pfSinBuffer, float fSampleRateInHz, float fInputFreq, float fInputInSecond, float fAmp){
    int sinLen = static_cast<int>(fInputInSecond * fSampleRateInHz);

    pfSinBuffer = new float[sinLen];

    for (int i = 0; i < sinLen; i++){
        pfSinBuffer[i] = fAmp * sin((2.F * PI * fInputFreq / fSampleRateInHz) * i);
    }

    return;
}

void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

