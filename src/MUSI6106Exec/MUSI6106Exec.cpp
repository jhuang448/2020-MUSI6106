
#include <iostream>
#include <ctime>
#include <cmath>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();
static void show_usage(char *argv);

void genSin(float*& sinBuffer, float fSampleRateInHz, float inputFreq, float inputInSecond, float fAmp = 1);
int    test_FIR();
int    test_zeros();

const float  PI_F=3.14159265358979f;

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath,
                            filterType;
    
    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFileInput = 0, *phAudioFileOutput = 0;

    CAudioFileIf::FileSpec_t stFileSpec;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType;
    
    float                   delayInSecond = 0, gain = 0;

    //showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 11) {
        show_usage(argv[0]);
        
        cout << endl << "Run tests..." << endl;
        
        // test 1
        test_FIR();

        // test 2
        
        // test 3
        
        // test 4
        test_zeros();
        
        // test 5
        
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
            filterType = argv[++i];
            if (filterType != "FIR" && filterType != "IIR"){
                cout << "Invalid filter type!" << endl;
                show_usage(argv[0]);
                return -1;
            }
        } else if ((arg == "-g") || (arg == "--gain")) {
            gain = std::stof(argv[++i]);
        } else if ((arg == "-d") || (arg == "--delay")) {
            delayInSecond = std::stof(argv[++i]);
        }
    }

//    sInputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep.wav"; //argv[1];
//    sOutputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep_fir.wav"; //argv[2];
//    filterType = "FIR"; //argv[3];
//    delayInSecond = 0.05;
//    gain = 0.1;
    
    if (filterType == "FIR") //FIR
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
    phCombFilter->init(combFilterType, 3, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
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

        cout << "\r" << "reading and writing";

        phCombFilter->process(ppfAudioData, ppfAudioData, iNumFrames);
        phAudioFileOutput->writeData(ppfAudioData, iNumFrames);
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

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

// test_FIR
// test FIR on one block of sinusoid, delay = half of the period
// the output should be zero
int test_FIR() // TODO: replace 1 with inumchannels
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;
    
    float                   **ppfSinBuffer = 0;    // generate sinusoid here
    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;
    
    int                     iNumChannels = 1;
    float                   fSampleRateInHz = 44100;
    float                   fInputFreq = 50, fInputInSecond = 3;
    float                   fGain = 1;
    float                   fDelayInSecond = 1 / fInputFreq / 2;

    int                     iDelayInSample = static_cast<int>(fDelayInSecond * fSampleRateInHz);
    int                     iNumBlocks = floor(fSampleRateInHz * fInputInSecond / kBlockSize);
    
    cout << "Start test 1..." << endl;

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
    phCombFilter->init(combFilterType, 1, fSampleRateInHz, iNumChannels);
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

    delete ppfAudioData[0];
    delete ppfAudioData;
    delete ppfAudioDataO[0];
    delete ppfAudioDataO;
    delete ppfSinBuffer[0];
    delete ppfSinBuffer;
    
    cout << "Test1 passed." << endl;
    
    return 0;
}

// test_zeros
// test FIR & IIR on one block of zero input
// the output should be zero
int test_zeros()
{
    static const int        kBlockSize = 1024;
    long long               iNumFrames = kBlockSize;

    float                   **ppfAudioData = 0, **ppfAudioDataO = 0;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::kCombFIR;   // FIR, IIR
    
    int                     iNumChannels = 1;
    float                   fSampleRateInHz = 44100;
    float                   fDelayInSecond = 0.00227, fGain = 1;         // freq: 441 Hz
    
    cout << "Start test 4..." << endl;

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
    phCombFilter->init(combFilterType, 1, fSampleRateInHz, iNumChannels);
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
    phCombFilter->init(combFilterType, 1, fSampleRateInHz, iNumChannels);
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
    
    delete ppfAudioData[0];
    delete ppfAudioData;
    delete ppfAudioDataO[0];
    delete ppfAudioDataO;
    
    cout << "Test4 passed." << endl;
    
    return 0;
}

void genSin(float*& pfSinBuffer, float fSampleRateInHz, float fInputFreq, float fInputInSecond, float fAmp){
    int sinLen = static_cast<int>(fInputInSecond * fSampleRateInHz);

    pfSinBuffer = new float[sinLen];

    for (int i = 0; i < sinLen; i++){
        pfSinBuffer[i] = fAmp * sin(2 * PI_F * fInputFreq / fSampleRateInHz * i);
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

