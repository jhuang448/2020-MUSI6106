# if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "ErrorDef.h"
#include "RingBuffer.h"
#include "Lfo.h"

/*!	\brief Class for the delay-based vibrato effect
*/
class CVibrato:
{
public:
	/*! version number */
    enum Version_t
    {
        kMajor,                         //!< major version number
        kMinor,                         //!< minor version number
        kPatch,                         //!< patch version number

        kNumVersionInts
    };

	/*! list of parameters for the vibrato */
    enum VibratoParam_t
    {
        kParamDelay,                    //!< delay in seconds
        kParamWidth,							//!< LFO amplitude in seconds
        kParamModFreq,						//!< LFO frequency in Hz

        kNumVibratoParams
    };

	/*! returns the current project version
    \param eVersionIdx specifies what version to return (kMajor, kMinor,...)
    \return const int
    */
    static const int  getVersion (const Version_t eVersionIdx);

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
    \param fWidthInS modulation width in seconds
    \param fModFreqInHz modulation frequency in Hz
    \return Error_t
    */
    Error_t init (float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels, float fWidthInS, float fModFreqInHz);
    
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

private:
	bool					m_bIsInitialized;	//!< internal bool to check whether the init function has been called
	
	CRingBuffer<float>		*m_pCRingBuffer;	//!< handle of the ring buffers
	CLfo					*m_pCLfo;			//!< handle of the LFO

	float					m_fDelayLengthInS;	//!< delay length in seconds
	float					m_fWidthInS;		//!< modulation width in seconds
	float					m_fModFreq;			//!< modulation frequency in Hz

	int 					m_iNumChannels;		//!< number of channels
	float					m_fSampleRate;		//!< audio sample rate in Hz
}





#endif // # if !defined(__Vibrato_hdr__)