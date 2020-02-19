
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "RingBuffer.h"
#include "Fft.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;
    
    clock_t                 time = 0;

    float                   **ppfAudioData = 0;             //!< audio read
    float                   *pfFftAudioData = 0;            //!< fft block
    CFft::complex_t         *pfSpectrum = 0;                //!< spectrum
    float                   *pfMag = 0;                     //!< magnitude
    
    CRingBuffer<float>      *phAudioBuffer = 0;             //!< Ring Buffer handler
    CFft                    *phFft = 0;                     //!< CFft

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    // fft cmd args
    int                     iBlockLength = 4096;
    int                     iHopLength = 2048;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        if (argc >= 3)
            iBlockLength = std::stoi(argv[2]);
        if (argc >= 4)
            iHopLength = std::stoi(argv[3]);
    }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    
    assert(stFileSpec.iNumChannels == 1);
    
    cout << "Computing FFT with block length = " << iBlockLength << " & hop length = " << iHopLength << " ..." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[iBlockLength];
    pfFftAudioData = new float[iBlockLength];

    //////////////////////////////////////////////////////////////////////////////
    // initialize ring buffer
    phAudioBuffer = new CRingBuffer<float>(iBlockLength);
    // initialize the fft
    CFft::createInstance(phFft);
    phFft->initInstance(iBlockLength);
    
    // num of frames to read each time
    long long iNumFrames = iHopLength;
    int iMagLength = phFft->getLength(CFft::Length_t::kLengthMagnitude);
    
    // allocate memory for spec and mag pointers
    pfSpectrum = new CFft::complex_t[iBlockLength];
    pfMag = new float[iMagLength];
    
    // start timing
    time = clock();
    
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file

    // deal with the first block
    long long iNumFrames_tmp = iBlockLength;
    
    // read
    phAudioFile->readData(ppfAudioData, iNumFrames_tmp);
    // write in buffer
    phAudioBuffer->putPostInc(ppfAudioData[0], iNumFrames_tmp);
    // read from buffer
    phAudioBuffer->get(pfFftAudioData, iBlockLength);
    // set reader idx
    phAudioBuffer->setReadIdx(phAudioBuffer->getReadIdx() + iHopLength);
    // do fft
    phFft->doFft(pfSpectrum, pfFftAudioData);
    // get mag
    phFft->getMagnitude(pfMag, pfSpectrum);

    for (int i = 0 ; i < iMagLength; i++)
        hOutputFile << pfMag[i] << '\t';
    hOutputFile << endl;

    while (!phAudioFile->isEof())
    {
        // read
        phAudioFile->readData(ppfAudioData, iNumFrames);
        // write in buffer
        phAudioBuffer->putPostInc(ppfAudioData[0], iNumFrames);
        // read from buffer
        phAudioBuffer->get(pfFftAudioData, iBlockLength);
        // set reader idx
        phAudioBuffer->setReadIdx(phAudioBuffer->getReadIdx() + iHopLength);
        // do fft
        phFft->doFft(pfSpectrum, pfFftAudioData);
        // get mag
        phFft->getMagnitude(pfMag, pfSpectrum);
        
        for (int i = 0 ; i < iMagLength; i++)
            hOutputFile << pfMag[i] << '\t';
        hOutputFile << endl;
    }

    cout << "\nreading/writing done in: \t" << (clock() - time)*1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CFft::destroyInstance(phFft);
    CAudioFileIf::destroy(phAudioFile);
    hOutputFile.close();
    
    delete [] pfMag;
    delete [] pfSpectrum;
    delete [] pfFftAudioData;

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

