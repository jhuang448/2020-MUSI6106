
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

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

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    // command: $ ./MUSI6106Exec audioFile txtFile
    sInputFilePath = argv[1]; //"/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep.wav";
    sOutputFilePath = argv[2]; //"/Users/caspia/Desktop/spring20/MUSI 6106/audio/sweep.txt";
 
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, phAudioFile->kFileRead, &stFileSpec);
    phAudioFile->getFileSpec(stFileSpec);
 
    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath, std::fstream::out);
    hOutputFile.precision(16);
    
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = (float**)malloc(2*sizeof(float*));
    ppfAudioData[0] = (float*)malloc(kBlockSize*sizeof(float));
    ppfAudioData[1] = (float*)malloc(kBlockSize*sizeof(float));
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    long long int numFrames = kBlockSize;
    while (!phAudioFile->isEof()){
        phAudioFile->readData(ppfAudioData, numFrames);
        for (int i = 0; i < numFrames; i++){
            hOutputFile << ppfAudioData[0][i] << '\t' << ppfAudioData[1][i] << endl;
        }
    }
    
    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    delete [] ppfAudioData[0];
    delete [] ppfAudioData[1];
    delete [] ppfAudioData;
    hOutputFile.close();
    phAudioFile->closeFile();
    CAudioFileIf::destroy(phAudioFile);

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

