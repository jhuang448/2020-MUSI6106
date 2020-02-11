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

Error_t CVibrato::init (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
{
	reset();

	// check input related parameters
	if (fMaxDelayLengthInS <= 0 ||
        fSampleRateInHz <= 0 ||
        iNumChannels <= 0)
        return kFunctionInvalidArgsError;

    m_iNumChannels = iNumChannels;
    m_fSampleRate = fSampleRateInHz;
    m_fMaxDelayLengthInS = fMaxDelayLengthInS;

    m_ppCRingBuffer = new CRingBuffer<float>*[m_iNumChannels];
    m_pCLfoReadIdx = new int[m_iNumChannels];
    
    for (int i = 0; i < m_iNumChannels; i++)
        m_ppCRingBuffer[i] = 0;

    m_bIsInitialized = true;

    return kNoError;
}

Error_t CVibrato::initRingBuffer ()
{
	if (!m_bIsInitialized)
		return kNotInitializedError;

    int iDelayInSamples = CUtil::float2int<int>(m_fDelayLengthInS * m_fSampleRate);
    int iWidthInSamples = CUtil::float2int<int>(m_fWidthInS * m_fSampleRate);
    int iBufferLength = 2 + iDelayInSamples + 2 * iWidthInSamples;
    for (int i = 0; i < m_iNumChannels; i++) {

    	if (m_ppCRingBuffer[i])
    		delete m_ppCRingBuffer[i];

        m_ppCRingBuffer[i] = new CRingBuffer<float>(iBufferLength);
        m_ppCRingBuffer[i]->setWriteIdx(iDelayInSamples);
    }

	return kNoError;
}


Error_t CVibrato::initLFO ()
{
	if (!m_bIsInitialized)
		return kNotInitializedError;
	
    int iWidthInSamples = CUtil::float2int<int>(m_fWidthInS * m_fSampleRate);
    int iLfoBufferLengthInSamples = max(CUtil::float2int<int>(m_fSampleRate / m_fModFreq), 1);
    float fModFreqInSamples = m_fModFreq / m_fSampleRate;

    if (m_pCLfo)
    	delete m_pCLfo;

    m_pCLfo = new CLfo(iLfoBufferLengthInSamples, static_cast<float>(iWidthInSamples), fModFreqInSamples);
    for (int i = 0; i < m_iNumChannels; i++) {
        m_pCLfoReadIdx[i] = m_pCLfo->getReadIdx();
    }
    
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

	delete m_pCLfoReadIdx;
	m_pCLfoReadIdx = 0;

	m_iNumChannels = 0;
	m_fSampleRate = 0;
	m_bIsInitialized = false;
	m_fMaxDelayLengthInS = 0;
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
        {
            m_fDelayLengthInS = fParamValue;
            if (m_fDelayLengthInS < 0 || m_fDelayLengthInS > m_fMaxDelayLengthInS)
                return kFunctionInvalidArgsError;
            break;
        }
		case kParamWidth:
        {
            m_fWidthInS = fParamValue;
            if (m_fWidthInS < 0 || m_fWidthInS > m_fDelayLengthInS)
                return kFunctionInvalidArgsError;
            break;
        }
		case kParamModFreq:
        {
            m_fModFreq = fParamValue;
            if (m_fModFreq < 0)
                return kFunctionInvalidArgsError;
            break;
        }
        default:
        	return kUnknownError;
	}
    
    return kNoError;
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
    	// retrieve LFO read idx
    	m_pCLfo->setReadIdx(m_pCLfoReadIdx[c]);

        for (int i = 0; i < iNumberOfFrames; i++)
        {
            m_ppCRingBuffer[c]->putPostInc(ppfInputBuffer[c][i]);
            fOffsetTmp = m_pCLfo->getPostInc();
            ppfOutputBuffer[c][i] = m_ppCRingBuffer[c]->get(-fOffsetTmp);
            m_ppCRingBuffer[c]->getPostInc();
        }

        // save LFO read idx
        m_pCLfoReadIdx[c] = m_pCLfo->getReadIdx();
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










