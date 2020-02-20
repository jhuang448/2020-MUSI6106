#if !defined(__Lfo_hdr__)
#define __Lfo_hdr__

#include <cassert>
#include <cmath>
#include "RingBuffer.h"

/*!	\brief Class for LFO
*/
class CLfo: public CRingBuffer<float>
{
public:
	/*! list of parameters for the LFO */
    enum LfoParam_t
    {
        kWidth,							//!< LFO amplitude in samples
        kModFreq,						//!< LFO modulation frequency in # samples (FreqInHz/SRInHz)

        kNumLfoParams
    };

    // explicit constructor
    /*! initializes the ringbuffer and fill the buffer with sinusoidal
    */
	explicit CLfo (int iBufferLengthInSamples, float fWidth, float fModeFreq): 
		CRingBuffer<float>(iBufferLengthInSamples),
		m_iBuffLength(iBufferLengthInSamples),
		m_fWidth(fWidth),
		m_fModFreq(fModeFreq)
	{
		assert(iBufferLengthInSamples > 0);

		writeSinusoidal();
	}

	// destructor: do nothing
	~CLfo ()
	{
	}

	/*! calls buffer reset, set parameters to 0
    \return void
    */
    void reset ()
   	{
   		CRingBuffer::reset();
   		m_iBuffLength = 0;
   		m_fWidth = 0;
   		m_fModFreq = 0;
   	}

   	/*! print LFO buffer status
   	*/
   	void printStatus()
   	{
   		printf("**************** Print LFO Status ****************\n");
   		printf("m_iBuffLength: %d m_fWidth: %f m_fModFreq: %f\n", m_iBuffLength, m_fWidth, m_fModFreq);
   		CRingBuffer<float>::printStatus();
   	}

private:
	// disable default constructor and copy constructor
	CLfo ();
	CLfo(const CLfo& that);

	/*! write sinusoidal to the buffer
	*/
	void writeSinusoidal()
	{
		for (int i = 1; i <= m_iBuffLength; i++) {
			CRingBuffer::putPostInc(m_fWidth * sin(i * 2 * M_PI * m_fModFreq));
		}
	}

	int 			m_iBuffLength;				//!< buffer length of LFO
	float 			m_fWidth;					//!< modulation width in samples
	float			m_fModFreq;					//!< modulation frequency in # samples
};

#endif // #if !defined(__Lfo_hdr__)
