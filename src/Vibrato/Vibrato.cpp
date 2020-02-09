#include "ErrorDef.h"
#include "Util.h"
#include "RingBuffer.h"
#include "Lfo.h"

static const char*  kCMyProjectBuildDate             = __DATE__;

const int  CVibrato::getVersion (const Version_t eVersionIdx)
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

const char* CVibrato::getBuildDate ()
{
	return kCMyProjectBuildDate;
}

Error_t CVibrato::create (CVibrato*& pCVibrato)
{
	pCVibrato = new pCVibrato();

	if (!pCVibrato)
		return kUnknownError;

	return kNoError;
}

Error_t CVibrato::destroy (CVibrato*& pCVibrato)
{
	if (!pCVibrato)
		return kUnknownError;

	pCVibrato->reset();

	delete pCVibrato;
	pCVibrato = 0;

	return kNoError;
}

Error_t CVibrato::init (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels, float fWidthInS, float fModFreqInHz)
{
	reset();

	// check input related parameters
	if (fMaxDelayLengthInS <= 0 ||
        fSampleRateInHz <= 0 ||
        iNumChannels <= 0)
        return kFunctionInvalidArgsError;

    // check LFO related parameters
    if (fMaxDelayLengthInS < fWidthInS ||
    	fWidthInS <= 0 ||
    	fModFreqInHz <= 0)
    	return kFunctionInvalidArgsError;

    m_iNumChannels = iNumChannels;
    m_fSampleRate = fSampleRateInHz;
    m_fDelayLengthInS = fMaxDelayLengthInS;
    m_fWidthInS = fWidthInS;
    m_fModFreq = fModFreqInHz;

    // initialize ring buffers
    int iDelayInSamples = CUtil::float2int<int>(fMaxDelayLengthInS * fSampleRateInHz);
    int iWidthInSamples = CUtil::float2int<int>(fWidthInS * fSampleRateInHz);
    int iBufferLength = 2 + iDelayInSamples + 2 * iWidthInSamples; // why multiply by 2?
    for (int i = 0; i < m_iNumChannels; i++)
    	m_pCRingBuffer[i] = new CRingBuffer(iBufferLength);

    // initialize LFO
    int iLfoBufferLengthInSamples = CUtil::float2int<int>(fSampleRateInHz / fModFreqInHz);
    float fModFreqInSamples = fModFreqInHz / fSampleRateInHz;
    m_pCLfo = new CLfo(iLfoBufferLengthInSamples, static_cast<float>(iWidthInSamples), fModFreqInSamples);

    m_bIsInitialized = true;

    return kNoError;
}


Error_t CVibrato::reset ()
{
	// delete ring buffers
	delete [] m_pCRingBuffer;
	m_pCRingBuffer = 0;

	// delete LFO
	delete m_pCLfo;
	m_pCLfo = 0;

	m_fSampleRate = 0;
	m_bIsInitialized = false;
	m_fDelayLengthInS = 0;
    m_fWidthInS = 0;
    m_fModFreq = 0;

	return kNoError;
}

Error_t CVibrato::setParam (VibratoParam_t eParam, float fParamValue)
{
	if (!m_bIsInitialized)
		return kNotInitializedError;

	switch (eParam) // reinitialize
	{
		case kParamDelay:
			return init(fParamValue, m_fSampleRate, m_iNumChannels, m_fWidthInS, m_fModFreq);
		case kParamWidth:
			return init(m_fDelayLengthInS, m_fSampleRate, m_iNumChannels, fParamValue, m_fModFreq);
		case kParamModFreq:
			return init(m_fDelayLengthInS, m_fSampleRate, m_iNumChannels, m_fWidthInS, fParamValue);
	}
}

float   CVibrato::getParam (VibratoParam_t eParam) const
{
	if (!m_bIsInitialized)
		return kNotInitializedError;

	switch (eParam)
	{
		case kParamDelay:
			return m_fDelayLengthInS;
		case kParamWidth:
			return m_fWidthInS;
		case kParamModFreq:
			return m_fModFreq;
	}
}

Error_t CVibrato::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
	//
	return kNoError;
}















