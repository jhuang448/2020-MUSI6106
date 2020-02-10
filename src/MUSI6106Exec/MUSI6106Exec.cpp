
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    CVibrato        *pCVibrato = 0;
    
    CVibrato::create(pCVibrato);
    pCVibrato->init(0.01, 44100, 2, 1.0/441, 441);
    pCVibrato->printStatus();
    
    return 0;

}


void     showClInfo()
{
    cout << "GTCMT MUSI6106 Executable" << endl;
    cout << "(c) 2014-2020 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

