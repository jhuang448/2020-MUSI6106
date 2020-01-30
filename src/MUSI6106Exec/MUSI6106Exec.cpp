
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
//#include "CombFilterIf.h"
#include "RingBuffer.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();
void    testRingBufferManualCheck();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    testRingBufferManualCheck();
    return 0;

}

void testRingBufferManualCheck()
{
    int                     iRingBuffLength = 5;
    
    CRingBuffer<float>      *pCRingBuffer = new CRingBuffer<float> (iRingBuffLength);
    
    pCRingBuffer->printBufferStatus();
    cout << "--------------put, putPostInc----------------" << endl;
    
    // put, put, put...
    pCRingBuffer->put(1);
    pCRingBuffer->putPostInc(2);
    pCRingBuffer->printBufferStatus();
    
    cout << "--------------get, getPostInc----------------" << endl;
    
    // get, get, get
    cout << pCRingBuffer->get() << endl;
    cout << pCRingBuffer->getPostInc() << endl;
    pCRingBuffer->printBufferStatus();
    
    cout << "--------------put, get----------------" << endl;
    pCRingBuffer->put(1);
    pCRingBuffer->get(); // should return error, since the pointer does not move
    
    pCRingBuffer->reset();
    
    cout << "--------------putPostInc x 6----------------" << endl;
    for (int i = 0; i <= iRingBuffLength; i++){
        pCRingBuffer->putPostInc(i+1);
        pCRingBuffer->printBufferStatus();
    }
    
    cout << "--------------getPostInc x 6----------------" << endl;
    for (int i = 0; i <= iRingBuffLength; i++){
        pCRingBuffer->getPostInc();
        pCRingBuffer->printBufferStatus();
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

