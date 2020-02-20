#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include <cassert>
#include <cstdio>

#include "UnitTest++.h"

#include "Synthesis.h"
#include "Vector.h"
#include "Vibrato.h"

SUITE(RingBuffer)
{
    struct RingBufferData
    {
        RingBufferData() :
            m_iRingBufferLength(5),
            m_iDataLength(17)
        {
            m_pCRingBuffer = new CRingBuffer<float> (m_iRingBufferLength);
            m_pDataInput = new float[m_iDataLength];
            m_pDataOutput = new float[m_iDataLength];
            CSynthesis::generateNoise(m_pDataInput, m_iDataLength, static_cast<float>(rand())/RAND_MAX*17000.F);
        }

        ~RingBufferData()
        {
            delete [] m_pDataInput;
            delete [] m_pDataOutput;
            delete m_pCRingBuffer;
            m_pCRingBuffer = 0;
            m_pDataInput = 0;
            m_pDataOutput = 0;
        }

        int                     m_iRingBufferLength;
        CRingBuffer<float>      *m_pCRingBuffer;

        int                     m_iDataLength;
        float                   *m_pDataInput;
        float                   *m_pDataOutput;
    };

    TEST_FIXTURE(RingBufferData, PutGetIdx)
    {
        m_pCRingBuffer->reset();
        
        float v = static_cast<float>(rand())/RAND_MAX*17000.F;
        int pos = static_cast<int> (RAND_MAX % m_iRingBufferLength);
        
        m_pCRingBuffer->setWriteIdx(pos);
        
        CHECK_EQUAL(pos, m_pCRingBuffer->getWriteIdx());
        
        m_pCRingBuffer->put(v);
        
        m_pCRingBuffer->setReadIdx(pos);
        
        CHECK_EQUAL(pos, m_pCRingBuffer->getReadIdx());
    
        CHECK_EQUAL(v, m_pCRingBuffer->get());
    }
    
    TEST_FIXTURE(RingBufferData, PutGetPostInc)
    {
        m_pCRingBuffer->reset();

        for (int i = 0; i < m_iDataLength; i++)
        {
            m_pCRingBuffer->putPostInc(m_pDataInput[i]);
            m_pDataOutput[i] =m_pCRingBuffer->getPostInc();
            CHECK_EQUAL(m_pCRingBuffer->getReadIdx(), m_pCRingBuffer->getWriteIdx());
        }

        CHECK_ARRAY_CLOSE(m_pDataInput, m_pDataOutput,m_iDataLength, 1e-5);
    }

    TEST_FIXTURE(RingBufferData, PutGetArray)
    {
        m_pCRingBuffer->reset();

        m_pCRingBuffer->putPostInc(m_pDataInput, m_iRingBufferLength);
        m_pCRingBuffer->getPostInc(m_pDataOutput, m_iRingBufferLength);

        CHECK_ARRAY_CLOSE(m_pDataInput, m_pDataOutput,m_iRingBufferLength, 1e-5);

        CHECK_EQUAL(m_pCRingBuffer->getReadIdx(), m_pCRingBuffer->getWriteIdx());
    }

    TEST_FIXTURE(RingBufferData, PutGetOverflow)
    {
        m_pCRingBuffer->reset();

        int iRemainDataLength = m_iDataLength;
        int i = 0;
        while (true) {
            if (iRemainDataLength <= m_iRingBufferLength) {
                m_pCRingBuffer->putPostInc(m_pDataInput+i, iRemainDataLength);
                break;
            }
            m_pCRingBuffer->putPostInc(m_pDataInput+i, m_iRingBufferLength);
            iRemainDataLength -= m_iRingBufferLength;
            i += m_iRingBufferLength;
        }

        m_pCRingBuffer->getPostInc(m_pDataOutput, iRemainDataLength);

        int iIdxNotOverwritten = m_iDataLength - (m_iDataLength % m_iRingBufferLength);

        CHECK_ARRAY_CLOSE(m_pDataInput + iIdxNotOverwritten, m_pDataOutput, (m_iDataLength % m_iRingBufferLength), 1e-5);
    }
    
    TEST_FIXTURE(RingBufferData, Frac)
    {
        m_pCRingBuffer->reset();

        for (int i = 0; i < m_iRingBufferLength; i++)
            m_pCRingBuffer->putPostInc(i * 1.F);

        for (int i = 0; i < 5; i++)
        {
            float v = fmod((static_cast<float>(rand())/RAND_MAX*17000.F), m_iRingBufferLength);
            m_pCRingBuffer->setReadIdx(0);
            CHECK_EQUAL(v, m_pCRingBuffer->get(v));
        }
        
        for (int i = 0; i < 5; i++)
        {
            float v = -fmod((static_cast<float>(rand())/RAND_MAX*17000.F), m_iRingBufferLength-1)-1;    // avoid interpolation between the last and the first value
            m_pCRingBuffer->setReadIdx(0);
            CHECK_EQUAL(m_iRingBufferLength + v, m_pCRingBuffer->get(v));
        }
    }
}

SUITE(Vibrato)
{
	struct VibratoData
	{
		VibratoData() :
			m_pCVibrato(0),
			m_ppfInputData(0),
            m_ppfOutputData(0),
            m_iDataLength(35131),
            m_fMaxDelayInS(3.F),
            m_iBlockLength(171),
            m_iNumChannels(3),
            m_fSampleRateInHz(8000),
            m_fDelayInS(.1F),
            m_fWidthInS(.05F),
            m_fModFreqInHz(10)
        {
        	CVibrato::create(m_pCVibrato);

        	m_ppfInputData  = new float*[m_iNumChannels];
            m_ppfOutputData = new float*[m_iNumChannels];
            m_ppfInputTmp   = new float*[m_iNumChannels];
            m_ppfOutputTmp  = new float*[m_iNumChannels];
            for (int i = 0; i < m_iNumChannels; i++)
            {
                m_ppfInputData[i]   = new float [m_iDataLength];
                CVectorFloat::setZero(m_ppfInputData[i], m_iDataLength);
                m_ppfOutputData[i]  = new float [m_iDataLength];
                CVectorFloat::setZero(m_ppfOutputData[i], m_iDataLength);            
            }
        }

        ~VibratoData()
        {
        	for (int i = 0; i < m_iNumChannels; i++)
            {
                delete [] m_ppfOutputData[i];
                delete [] m_ppfInputData[i];
            }
            delete [] m_ppfOutputTmp;
            delete [] m_ppfInputTmp;
            delete [] m_ppfOutputData;
            delete [] m_ppfInputData;

            CVibrato::destroy(m_pCVibrato);
        }

        void TestProcess()
        {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {
                int iNumFrames = std::min(iNumFramesRemaining, m_iBlockLength);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c]    = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                    m_ppfOutputTmp[c]   = &m_ppfOutputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCVibrato->process(m_ppfInputTmp, m_ppfOutputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        void TestProcessInplace() 
        {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {
                int iNumFrames = std::min(iNumFramesRemaining, m_iBlockLength);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c]    = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCVibrato->process(m_ppfInputTmp, m_ppfInputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        CVibrato  	*m_pCVibrato;
        float       **m_ppfInputData,
                    **m_ppfOutputData,
                    **m_ppfInputTmp,
                    **m_ppfOutputTmp;
        int     m_iDataLength;
        float   m_fMaxDelayInS;
        int     m_iBlockLength;
        int     m_iNumChannels;
        float   m_fSampleRateInHz;
        float   m_fDelayInS;
        float   m_fWidthInS;
        float   m_fModFreqInHz;
	};

	TEST_FIXTURE(VibratoData, ZeroInput)
	{
		m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, m_fWidthInS, m_fModFreqInHz);
        
	    TestProcess();

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-5);

        m_pCVibrato->reset();
	}

	TEST_FIXTURE(VibratoData, ZeroModAmp)
	{
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateNoise (m_ppfInputData[c], m_iDataLength, static_cast<float>(rand())/RAND_MAX*17000.F);
        
        m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, 0, m_fModFreqInHz);
        
        TestProcess();
        
        int iDelayInSamples = CUtil::float2int<int>(m_fDelayInS * m_fSampleRateInHz);
        
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c] + iDelayInSamples, m_iDataLength-iDelayInSamples, 1e-5);
	}

	TEST_FIXTURE(VibratoData, DCInput)
	{
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateDc (m_ppfInputData[c], m_iDataLength, static_cast<float>(rand())/RAND_MAX*17000.F);
        
        m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, m_fWidthInS, m_fModFreqInHz);
        
        TestProcess();
        
        int iDelayInSamples = CUtil::float2int<int>(m_fDelayInS * m_fSampleRateInHz) + CUtil::float2int<int>(m_fWidthInS * m_fSampleRateInHz);
        
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c] + iDelayInSamples, m_iDataLength - iDelayInSamples, 1e-3);
	}

	TEST_FIXTURE(VibratoData, VaryingBlocksize)
	{
		for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateSine (m_ppfInputData[c], 387.F, m_fSampleRateInHz, m_iDataLength, .8F, static_cast<float>(c*M_PI_2));

        m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, m_fWidthInS, m_fModFreqInHz);

	    TestProcess();

	    m_pCVibrato->reset();
	    m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, m_fWidthInS, m_fModFreqInHz);
        
	    {
            int iNumFramesRemaining = m_iDataLength;
            while (iNumFramesRemaining > 0)
            {

                int iNumFrames = std::min(static_cast<float>(iNumFramesRemaining), static_cast<float>(rand())/RAND_MAX*17000.F);

                for (int c = 0; c < m_iNumChannels; c++)
                {
                    m_ppfInputTmp[c]    = &m_ppfInputData[c][m_iDataLength - iNumFramesRemaining];
                }
                m_pCVibrato->process(m_ppfInputTmp, m_ppfInputTmp, iNumFrames);

                iNumFramesRemaining -= iNumFrames;
            }
        }

        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c], m_iDataLength, 1e-3);
	}

	TEST_FIXTURE(VibratoData, ZeroModFreq)
	{
        for (int c = 0; c < m_iNumChannels; c++)
            CSynthesis::generateNoise (m_ppfInputData[c], m_iDataLength, static_cast<float>(rand())/RAND_MAX*17000.F);
        
        m_pCVibrato->init(m_fMaxDelayInS, m_fSampleRateInHz, m_iNumChannels, m_fDelayInS, m_fWidthInS, 0);
        
        TestProcess();
        
        int iDelayInSamples = CUtil::float2int<int>(m_fDelayInS * m_fSampleRateInHz);
        
        for (int c = 0; c < m_iNumChannels; c++)
            CHECK_ARRAY_CLOSE(m_ppfInputData[c], m_ppfOutputData[c] + iDelayInSamples, m_iDataLength-iDelayInSamples, 1e-5);
	}
}

#endif //WITH_TESTS
