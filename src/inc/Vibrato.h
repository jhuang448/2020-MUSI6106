# if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Lfo.h"

/*
    Following the CCombFilterIf class's design

    m_ppCRingBuffer:
        CVibrato class accesses its own ring buffers (one for each channel) via pointers.
 
    m_pCLfo:
        CLFO is a separate class derived from CRingBuffer. CVibrato class has a member pointer pointing to an LFO.
 
    m_pCLfoReadIdx:
        Since different channels share the same LFO, they need to save the read idx for each channel.

    CVibrato class has three init functions:
    init:              set parameters according to the input audio (number of channels, sampling rate)
    initRingBuffer:    init the internel ringbuffers according to [delayLength, modWidth]
    initLFO:           init the LFO according to [delayLength, modWidth, modFreq]
    The last two cannot be done within the first one because they need vibrato parameters.
    To set these parameters one needs to call setParam
    (update: the fourth init function doing the init, setParams, initRingBuffer and initLFO)

    To use a CVibrato:
    (0. read audio)
    1. create
    2. init
    3. set parameters x 3(delay length, modulation amplitude and frequency)
    4. initRingBuffer
    5. initLFO
    6. process block by block
    7. destroy
    (update: another init function wraps up step 2 ~ 5)
 */

/*!	\brief Class for the delay-based vibrato effect
*/
class CVibrato
{
public:
	/*! list of parameters for the vibrato */
    enum VibratoParam_t
    {
        kParamDelay,                    //!< delay in seconds
        kParamWidth,							//!< LFO amplitude in seconds
        kParamModFreq,						//!< LFO frequency in Hz

        kNumVibratoParams
    };

    /*! returns the date of the build
    \return const char*
    */
    static const char* getBuildDate ();

    /*! creates a new vibrato instance
    \param pCVibrato pointer to the new class
    \return Error_t
    */
    static Error_t create (CVibrato*& pCVibrato);
    
    /*! destroys a vibrato instance
    \param pCVibrato pointer to the class to be destroyed
    \return Error_t
    */
    static Error_t destroy (CVibrato*& pCVibrato);
    
    /*! initializes a vibrato instance
    \param fMaxDelayLengthInS maximum allowed delay in seconds
    \param fSampleRateInHz sample rate in Hz
    \param iNumChannels number of audio channels
    \return Error_t
    */
    Error_t init (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels);
    
    /*! initializes a vibrato instance, with all parameters ready
    \param fMaxDelayLengthInS maximum allowed delay in seconds
    \param fSampleRateInHz sample rate in Hz
    \param iNumChannels number of audio channels
    \param fDelayLengthInS delay in seconds
    \param fWidthInS modulation width in seconds
    \param fModFreq modulation frequency in Hz
    \return Error_t
    */
    Error_t init (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels, float fDelayLengthInS, float fWidthInS, float fModFreqInHz);
    
    /*! initializes the ring buffers
    */
    Error_t initRingBuffer ();

    /*! initializes the LFO
    */
    Error_t initLFO ();
    
    /*! resets the internal variables (requires new call of init)
    \return Error_t
    */
    Error_t reset ();

    /*! sets a vibrato parameter
    \param eParam what parameter (see ::VibratoParam_t)
    \param fParamValue value of the parameter
    \return Error_t
    */
    Error_t setParam (VibratoParam_t eParam, float fParamValue);
    
    /*! return the value of the specified parameter
    \param eParam
    \return float
    */
    float   getParam (VibratoParam_t eParam) const;
    
    /*! processes one block of audio
    \param ppfInputBuffer input buffer [numChannels][iNumberOfFrames]
    \param ppfOutputBuffer output buffer [numChannels][iNumberOfFrames]
    \param iNumberOfFrames buffer length (per channel)
    \return Error_t
    */
    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);

    /*! prints current status
    */
    void printStatus();

private:

	bool					m_bIsInitialized;	    //!< internal bool to check whether both ring buffers and LFO are initialized

	CRingBuffer<float>		**m_ppCRingBuffer;	    //!< handle of the ring buffers
	CLfo					*m_pCLfo;			    //!< handle of the LFO
    int                     *m_pCLfoReadIdx;        //!< handle of the LFO read idx for each channel

    float                   m_fMaxDelayLengthInS;   //!< delay length in seconds
	float					m_fDelayLengthInS;	    //!< max delay length in seconds
	float					m_fWidthInS;		    //!< modulation width in seconds
	float					m_fModFreq;			    //!< modulation frequency in Hz

	int 					m_iNumChannels;		    //!< number of channels
	float					m_fSampleRate;		    //!< audio sample rate in Hz
};

#endif // # if !defined(__Vibrato_hdr__)
