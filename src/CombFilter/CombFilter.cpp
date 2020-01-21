#include <cassert>
#include <iostream>

#include "Util.h"
#include "Vector.h"

#include "CombFilter.h"

using namespace std;


CCombFilterBase::~CCombFilterBase()
{
	for (int i = 0; i < this->iNumChannels; i++)
		delete this->delayBuffer[i];
    delete this->delayStart;
	delete this->delayBuffer;
}

// get filter type
int CCombFilterBase::getFilterType() const
{
	return static_cast<int>(this->eFilterType);
}

// initialize a comb filter instance
Error_t CCombFilterBase::init (int eFilterType, float maxDelayLength, int iNumChannels)
{
	this->eFilterType = eFilterType;
	this->maxDelayLength = static_cast<int>(maxDelayLength);
	this->iNumChannels = iNumChannels;
	this->delayBuffer = new float*[iNumChannels];
    this->delayStart = new int[iNumChannels];
    
    // fill zeros (it really matters...)
    for (int i = 0; i < this->iNumChannels; i++){
		this->delayBuffer[i] = new float[this->maxDelayLength];
        for (int j = 0; j < this->maxDelayLength; j++)
            this->delayBuffer[i][j] = 0;
        this->delayStart[i] = 0;
    }
	return kNoError;
}

// sets a comb filter parameter
// if kParamDelay: cast float to int
Error_t CCombFilterBase::setParam (int eParam, float fParamValue)
{
	switch (eParam) {
		case kParamGain: 	this->gain = fParamValue;							break;
		case kParamDelay: 	this->delayLength = min(static_cast<int>(fParamValue), this->maxDelayLength); 	break;
	}
	return kNoError;
}

// get a comb filter parameter
// if kParamDelay: cast int to float
float   CCombFilterBase::getParam (int eParam) const
{
	float ret = 0;
	switch (eParam) {
		case kParamGain: 	ret = this->gain;								break;
        case kParamDelay: 	ret = static_cast<float>(this->delayLength);		break;
	}
	return ret;
}

// process one block of audio (multiple channels)
Error_t CCombFilterBase::processChannels(float **ppfAudioDataIn, float **ppfAudioDataOut, int iNumFrames)
{
	// iterate over channels
	for (int c = 0; c < this->iNumChannels; c++) {

		// process one channel
		float cur = 0;
        float *pfAudioDataInCur = ppfAudioDataIn[c], *pfAudioDataOutCur = ppfAudioDataOut[c];
		
		for (int i = 0; i < iNumFrames; i++) {

            if (!pfAudioDataOutCur || !pfAudioDataInCur)
                return kMemError;
            
			cur = *pfAudioDataInCur;
            
            if (this->delayLength != 0){
                // update output
                *pfAudioDataOutCur = cur + this->gain * this->delayBuffer[c][this->delayStart[c]];
                // update buffer
                if (this->eFilterType == kCombFIR)
                    this->delayBuffer[c][this->delayStart[c]] = cur;
                else
                    this->delayBuffer[c][this->delayStart[c]] = ppfAudioDataOut[c][i];

                this->delayStart[c] = (this->delayStart[c] + 1) % this->delayLength;
            }
            else    // copy input to output
                *pfAudioDataOutCur = cur;
            
            // move pointers
            pfAudioDataInCur++;
            pfAudioDataOutCur++;
        }
	}

	return kNoError;
}

void CCombFilterBase::printStatus(int level)
{
	cout << "------------Print CCombFilterBase Status:------------" << endl;
	if (level >= 0){
		cout << "eFilterType: " << this->eFilterType << endl;
		cout << "iNumChannels: " << this->iNumChannels << endl;
		cout << "maxDelayLength: " << this->maxDelayLength << endl;
	}

	if (level >= 1){
		cout << "delay in samples: " << this->delayLength << endl;
		cout << "gain: " << this->gain << endl;
	}

	if (level >= 2){
		if (!delayBuffer)
			cout << "delayBuffer not initialized..." << endl;
		cout << "delayStart: " << this->delayStart[0] << endl;
	}
	return;
}
