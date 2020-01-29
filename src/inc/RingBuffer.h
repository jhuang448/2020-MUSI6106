#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer
{
public:
    explicit CRingBuffer (int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples)
    {
        assert(iBufferLengthInSamples > 0);

        // allocate and init
        pRingBuffer = new T[m_iBuffLength];
        iReadIdx = 0;
        iWriteIdx = 0;
    }

    virtual ~CRingBuffer ()
    {
        // free memory
        delete pRingBuffer;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc (T tNewValue)
    {
        put(tNewValue);
        iWriteIdx++;
    }

    /*! add new values of type T to write index and increment write index
    \param ptNewBuff: new values
    \param iLength: number of values
    \return void
    */
    void putPostInc (const T* ptNewBuff, int iLength)
    {
        put(ptNewBuff, iLength);
        iWriteIdx = (iWriteIdx + iLength) % m_iBuffLength;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        pRingBuffer[iWriteIdx] = tNewValue;
    }

    /*! add new values of type T to write index
    \param ptNewBuff: new values
    \param iLength: number of values
    \return void
    */
    void put(const T* ptNewBuff, int iLength)
    {
        int iWriteIdxCur = iWriteIdx;
        for (int i = 0; i < iLength; i++){
            pRingBuffer[iWriteIdxCur++] = ptNewBuff[i];
            iWriteIdxCur = (iWriteIdxCur + 1) % m_iBuffLength;
        }
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc ()
    {
        return get(0.0);
    }

    /*! return the values starting at the current read index and increment the read pointer
    \param ptBuff: pointer to where the values will be written
    \param iLength: number of values
    \return void
    */
    void getPostInc (T* ptBuff, int iLength)
    {
        get(ptBuff, iLength);
        iReadIdx = (iReadIdx + iLength) % m_iBuffLength;
    }

    /*! return the value at the current read index
    \param fOffset: read at offset from read index
    \return float the value from the read index
    */
    T get (float fOffset = 0.f) const
    {
        int iReadPosi = iReadIdx + static_cast<int>(fOffset);
        return pRingBuffer[iReadPosi];
    }

    /*! return the values starting at the current read index
    \param ptBuff to where the values will be written
    \param iLength: number of values
    \return void
    */
    void get (T* ptBuff, int iLength) const
    {
        int iReadIdxCur = iReadIdx;
        for (int i = 0; i < iLength; i++){
            ptBuff[iReadIdxCur++] = pRingBuffer[i];
            iReadIdxCur = (iReadIdxCur + 1) % m_iBuffLength;
        }
    }
    
    /*! set buffer content and indices to 0
    \return void
    */
    void reset ()
    {
        memset(pRingBuffer, 0, m_iBuffLength * sizeof(T));
        iReadIdx = 0;
        iWriteIdx = 0;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx () const
    {
        return iWriteIdx;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx (int iNewWriteIdx)
    {
        if (iNewWriteIdx >= 0 && iNewWriteIdx < m_iBuffLength)
            iWriteIdx = iNewWriteIdx;
        else
            cout << "Trying to set invalid write index!" << endl;
        return;
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx () const
    {
        return iReadIdx;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx (int iNewReadIdx)
    {
        if (iNewReadIdx >= 0 && iNewReadIdx < m_iBuffLength)
            iReadIdx = iNewReadIdx;
        else
            cout << "Trying to set invalid read index!" << endl;
        return;
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer () const
    {
        return (iReadIdx - iWriteIdx) % m_iBuffLength;
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength () const
    {
        return m_iBuffLength;
    }
private:
    CRingBuffer ();
    CRingBuffer(const CRingBuffer& that);

    int m_iBuffLength;              //!< length of the internal buffer
    T* pRingBuffer;
    int iReadIdx;
    int iWriteIdx;
};
#endif // __RingBuffer_hdr__
