
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"

using std::cout;
using std::endl;

static void show_usage(char *argv);

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
	// AudioIO parameters
	std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

	static const int        kBlockSize = 1024;

	float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFileInput = 0, *phAudioFileOutput = 0;

    CAudioFileIf::FileSpec_t stFileSpec;

    // Vibrato parameters
    float                   fMaxDelayInS = 3.F;
    float					fDelayInS = 0;
    float					fWidthInS = 0;
    float					fSampleRateInHz = 0;
    int 					iNumChannels = 0;
    float					fModFreqInHz = 0;
    CVibrato        		*pCVibrato = 0;
    
    //sInputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/saxophone.wav";
    //sOutputFilePath = "/Users/caspia/Desktop/spring20/MUSI 6106/audio/saxophone_vibrato.wav";

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    showClInfo();
    if (argc < 5) {
        show_usage(argv[0]);
        return -1;
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
        } else if ((arg == "-d") || (arg == "--delay")) {
            fDelayInS = std::stof(argv[++i]);
            if (fDelayInS > fMaxDelayInS || fDelayInS < 0) {
                cout << "Invalid delay length!" << endl;
                return -1;
            }
        } else if ((arg == "-w") || (arg == "--witdth")) {
            fWidthInS = std::stof(argv[++i]);
            if (fWidthInS > fDelayInS || fWidthInS < 0) {
                cout << "Invalid modulation width!" << endl;
                return -1;
            }
        } else if ((arg == "-f") || (arg == "--freq")) {
            fModFreqInHz = std::stof(argv[++i]);
            if (fModFreqInHz < 0) {
                cout << "Invalid modulation frequency!" << endl;
                return -1;
            }
        }
        else{
            cout << "Invalid input arguments." << endl;
            show_usage(argv[0]);
            return -1;
        }
    }
    
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

    // get sampling rate and number of channels
    fSampleRateInHz = stFileSpec.fSampleRateInHz;
    iNumChannels = stFileSpec.iNumChannels;

    //////////////////////////////////////////////////////////////////////////////
    // initialize vibrato
    CVibrato::create(pCVibrato);
    pCVibrato->init(fMaxDelayInS, fSampleRateInHz, iNumChannels, fDelayInS, fWidthInS, fModFreqInHz);

    //pCVibrato->printStatus();

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    int numChannels = stFileSpec.iNumChannels;
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < numChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and process
    long long int iNumFrames = kBlockSize;
    while (!phAudioFileInput->isEof()){
        phAudioFileInput->readData(ppfAudioData, iNumFrames);
        pCVibrato->process(ppfAudioData, ppfAudioData, iNumFrames);
        //pCVibrato->printStatus();
        phAudioFileOutput->writeData(ppfAudioData, iNumFrames);
    }

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFileInput);
    CAudioFileIf::destroy(phAudioFileOutput);
    CVibrato::destroy(pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    return 0;
}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

static void show_usage(char *argv)
{
    std::cerr << "Usage: " << argv[0] << " <option(s)> "
    << "Options:\n"
    << "\t-h,--help\t\tShow this help message\n"
    << "\t-i,--input\tInput wave file\n"
    << "\t-o,--output\tOutput wave file\n"
    << "\t-d,--delay\tdelay in second\n"
    << "\t-w,--width\tmodulation width\n"
    << "\t-f,--freq\tmodulation frequency\n"
    << std::endl;
}
