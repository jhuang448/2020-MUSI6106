#if !defined(__Lfo_hdr__)
#define __Lfo_hdr__

#include <cassert>
#include <cmath>
#include "RingBuffer.h"

const double PI  =3.141592653589793238463;

/*!	\brief Class for LFO
*/
class CLfo: private CRingBuffer<float> // inheritance is fine because the Lfo only need one buffer
{
public:
	/*! list of parameters for the LFO */
    enum LfoParam_t
    {
        kWidth,							//!< LFO amplitude in sample
        kModFreq,						//!< LFO modulation frequency in # samples (FreqInHz/SRInHz)

        kNumLfoParams
    };

    // explicit constructor
    // initialize the ringbuffer and fill the buffer with sinusoidal
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

	/*! call buffer reset, set parameters to 0
    \return void
    */
    void reset ()
   	{
   		CRingBuffer::reset();
   		m_iBuffLength = 0;
   		m_fWidth = 0;
   		m_fModFreq = 0;
   	}

private:
	// disable default constructor and copy constructor
	CLfo ();
	CLfo(const CLfo& that);

	/*! write sinusoidal to the buffer
	*/
	void writeSinusoidal()
	{
		for (int i = 0; i < m_iBuffLength; i++) {
			CRingBuffer::putPostInt(m_fWidth * sin(i * 2 * PI * m_fModeFreq));
		}
	}

	int 			m_iBuffLength;
	float 			m_fWidth;
	float			m_fModFreq;
}



#endif // #if !defined(__Lfo_hdr__)