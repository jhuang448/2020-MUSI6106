
// standard headers
#include<math.h>
#include<iostream>

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilter.h"
#include "CombFilterIf.h"

using namespace std;

static const char*  kCMyProjectBuildDate             = __DATE__;

CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_pCCombFilter(0),
    m_fSampleRate(0)
{
    // this never hurts
    this->reset ();
}


CCombFilterIf::~CCombFilterIf ()
{
    this->reset ();
}

const int  CCombFilterIf::getVersion (const Version_t eVersionIdx)
{
    int iVersion = 0;

    switch (eVersionIdx)
    {
    case kMajor:
        iVersion    = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion    = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion    = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion    = -1;
        break;
    }

    return iVersion;
}
const char*  CCombFilterIf::getBuildDate ()
{
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create( CCombFilterIf*& pCCombFilter)
{
    pCCombFilter = new CCombFilterIf();
    return kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilter)
{
    // delete CCombFilterBase
    delete pCCombFilter;
    pCCombFilter = 0;
    return kNoError;
}

Error_t CCombFilterIf::init( CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels )
{
    float maxDelayLength = floor(fMaxDelayLengthInS * fSampleRateInHz);
    this->m_fSampleRate = fSampleRateInHz;

    // initialize CCombFilterBase here
    if (eFilterType == kCombFIR)
        this->m_pCCombFilter = new CCombFilterFIR();
    else if (eFilterType == kCombIIR)
        this->m_pCCombFilter = new CCombFilterIIR();
    
    this->m_pCCombFilter->init(maxDelayLength, iNumChannels);

    this->m_bIsInitialized = true;
    
    return kNoError;
}

Error_t CCombFilterIf::reset ()
{
    if (m_bIsInitialized)
        delete m_pCCombFilter;
    m_bIsInitialized = false;
    m_fSampleRate = 0;

    return kNoError;
}

Error_t CCombFilterIf::process( float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames )
{
    return this->m_pCCombFilter->processChannels(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
}

Error_t CCombFilterIf::setParam( FilterParam_t eParam, float fParamValue )
{
    if (eParam == kParamDelay)  // convert delay in seconds to delay in samples
        fParamValue *= this->m_fSampleRate;
    this->m_pCCombFilter->setParam(eParam, fParamValue);

    return kNoError;
}

float CCombFilterIf::getParam( FilterParam_t eParam ) const
{
    float fParamValue = this->m_pCCombFilter->getParam(eParam);
    if (eParam == kParamDelay)  // convert delay in seconds to delay in samples
        fParamValue /= this->m_fSampleRate;
    return fParamValue;
}

void CCombFilterIf::printStatus()
{
    cout << "------------Print CCombFilterIf Status:------------" << endl;
    if (!m_bIsInitialized)
        cout << "Not initialized..." << endl;
    else{
        cout << "sample rate: " << this->m_fSampleRate << endl;
        if (!this->m_pCCombFilter)
            cout << "CCombFilter not initialized..." << endl;
        else
            this->m_pCCombFilter->printStatus();
    }
    return;
}
