#include "ErrorDef.h"
#include "Util.h"
#include "RingBuffer.h"
#include "Lfo.h"

#include "Vibrato.h"

static const char*  kCMyProjectBuildDate             = __DATE__;

const char* CVibrato::getBuildDate ()
{
	return kCMyProjectBuildDate;
}

Error_t CVibrato::create (CVibrato*& pCVibrato)
{
	pCVibrato = new CVibrato();

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

    m_ppCRingBuffer = new CRingBuffer<float>*[m_iNumChannels];

    // initialize ring buffers
    int iDelayInSamples = CUtil::float2int<int>(fMaxDelayLengthInS * fSampleRateInHz);
    int iWidthInSamples = CUtil::float2int<int>(fWidthInS * fSampleRateInHz);
    int iBufferLength = 2 + iDelayInSamples + 2 * iWidthInSamples; // why multiply by 2?
    for (int i = 0; i < m_iNumChannels; i++) {
    	m_ppCRingBuffer[i] = new CRingBuffer<float>(iBufferLength);
    	m_ppCRingBuffer[i]->setWriteIdx(1 + iDelayInSamples);
    }

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
	for (int i = 0; i < m_iNumChannels; i++)
    	delete m_ppCRingBuffer[i];
	delete [] m_ppCRingBuffer;
	m_ppCRingBuffer = 0;

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
        default:
        	return kUnknownError;
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
		default:
			return kUnknownError;
	}
}

Error_t CVibrato::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
	float fOffsetTmp = 0;
	for(int c = 0; c < m_iNumChannels; c++)
    {
        for (int i = 0; i < iNumberOfFrames; i++)
        {
        	// update delay line
            m_ppCRingBuffer[c]->putPostInc(ppfInputBuffer[c][i]);
            m_ppCRingBuffer[c]->setReadIdx(1);
            // get offset
            fOffsetTmp = m_pCLfo->getPostInc();
            ppfOutputBuffer[c][i] = m_ppCRingBuffer[c]->get(-fOffsetTmp);
        }
    }
	return kNoError;
}

void CVibrato::printStatus()
{
	printf("**************** Print Vibrato Status ****************\n");
	printf("m_bIsInitialized: %d m_fSampleRate: %f m_iNumChannels: %d\n", m_bIsInitialized, m_fSampleRate, m_iNumChannels);
	printf("m_fDelayLengthInS: %f m_fWidthInS: %f m_fModFreq: %f\n", m_fDelayLengthInS, m_fWidthInS, m_fModFreq);
	if (m_bIsInitialized) {
		// print the buffer of the first channel
		m_ppCRingBuffer[0]->printStatus();
		// print the LFO
		m_pCLfo->printStatus();
	}

}










