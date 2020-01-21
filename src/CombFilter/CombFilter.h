#if !defined(__CombFilter_hdr__)
#define __CombFilter_hdr__

//#include "CombFilterIf.h"
#include "ErrorDef.h"

/* \brief apply comb filter (FIR & IIR) to the audio stream
*/
class CCombFilterBase
{
public:

	/*! feedforward or recursive comb filter */
    enum CombFilterType_t
    {
        kCombFIR,
        kCombIIR
    };

    /*! list of parameters for the comb filters */
    enum FilterParam_t
    {
        kParamGain,                     //!< gain as factor (usually -1...1)
        kParamDelay,                    //!< delay in seconds for specification of comb width

        kNumFilterParams
    };

    ~CCombFilterBase ();

	int getFilterType() const;

	// initialize a comb filter instance
	Error_t init (int eFilterType, float maxDelayLength, int iNumChannels);

	// sets a comb filter parameter
	Error_t setParam (int eParam, float fParamValue);

	// get a comb filter parameter
	float   getParam (int eParam) const;

	// process one block of audio (multiple channels)
	Error_t processChannels (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);
    
	// process one block of audio (one channel)
    Error_t processOneChannel (float *pfInputBuffer, float *pfOutputBuffer, int iNumberOfFrames);


    void printStatus (int level = 2);

private:
	// members initialized in init()
	int 				eFilterType;
	int 				iNumChannels;
    int 				maxDelayLength;

	// members set in setParam
	int					delayLength;
	float				gain;

	// members used in computation
	float				**delayBuffer;		// delay buffer: space allocated in the construction function
	int					*delayStart;			// start position in the buffer for the current delay
};

#endif //__CombFilter_hdr__
