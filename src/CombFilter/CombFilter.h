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

    virtual ~CCombFilterBase ();

	// initialize a comb filter instance
	Error_t init (float maxDelayLength, int iNumChannels);

	// sets a comb filter parameter
	Error_t setParam (int eParam, float fParamValue);

	// get a comb filter parameter
	float   getParam (int eParam) const;

	// process one block of audio (multiple channels)
	// realized in derived classes: the only difference between FIR & IIR is what to write into the buffer
	virtual Error_t processChannels (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) = 0;
    
	virtual int getFilterType() = 0;

    void printStatus (int level = 2);

protected:
	// members initialized in init()
	int 				iNumChannels;
    int 				maxDelayLength;

	// members set in setParam
	int					delayLength;
	float				gain;

	// members used in computation
	float				**delayBuffer;		// delay buffer: space allocated in the construction function
	int					*delayStart;			// start position in the buffer for the current delay
};

// create two derived classes to avoid checking the filter type when processing every frame
class CCombFilterFIR: public CCombFilterBase
{
public:
	int getFilterType();
	Error_t processChannels (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);
	//~CCombFilterFIR();
};

class CCombFilterIIR: public CCombFilterBase
{
public:
	int getFilterType();
	Error_t processChannels (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);   
	//~CCombFilterIIR();
};

#endif //__CombFilter_hdr__
