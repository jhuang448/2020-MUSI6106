#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>

using namespace std;

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
        memset(pRingBuffer, 0, m_iBuffLength * sizeof(T));
        iReadIdx = 0;
        iWriteIdx = 0;
        isFull = false;
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
        if (m_iBuffLength - getNumValuesInBuffer() == 0){
            cout << "Not enough space in the buffer!" << endl;
            return;
        }
        put(tNewValue);
        iWriteIdx = (iWriteIdx + 1) % m_iBuffLength;
        if (iWriteIdx == iReadIdx) // write pointer reaches the read pointer
            isFull = true; // the buffer is full
    }

    /*! add new values of type T to write index and increment write index
    \param ptNewBuff: new values
    \param iLength: number of values
    \return void
    */
    void putPostInc (const T* ptNewBuff, int iLength)
    {
        for (int i = 0; i < iLength; i++)
            putPostInc(ptNewBuff[i]);
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
            put(pRingBuffer + (iWriteIdxCur++ % m_iBuffLength), ptNewBuff[i]);
        }
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc ()
    {
        if (getNumValuesInBuffer() == 0){
            cout << "Not enough values in the buffer!" << endl;
            return -1;
        }
        if (iReadIdx == iWriteIdx) // current a full buffer
            isFull = false;     // read point increment; releasing one slot
        int v = get(iReadIdx);
        iReadIdx = (iReadIdx + 1) % m_iBuffLength;
        return v;
    }

    /*! return the values starting at the current read index and increment the read pointer
    \param ptBuff: pointer to where the values will be written
    \param iLength: number of values
    \return void
    */
    void getPostInc (T* ptBuff, int iLength)
    {
        for (int i = 0; i < iLength; i++){
            ptBuff[i] = getPostInc();
        }
    }

    /*! return the value at the current read index
    \param fOffset: read at offset from read index
    \return float the value from the read index
    */
    T get (float fOffset = 0.f) const
    {
        if (getNumValuesInBuffer() <= static_cast<int>(fOffset)){
            cout << "Not enough values in the buffer!" << endl;
            return -1;
        }
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
            ptBuff[i] = get(pRingBuffer + (iReadIdxCur++ % m_iBuffLength));
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
        if (isFull)
            return m_iBuffLength;
        else
            return (iWriteIdx - iReadIdx + m_iBuffLength) % m_iBuffLength;
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength () const
    {
        return m_iBuffLength;
    }

    void printBufferStatus()
    {
        cout << "Buffer Length: " << m_iBuffLength << endl;
        cout << "isFull: " << isFull << " valid values: " << getNumValuesInBuffer() << endl;
        for (int i = 0; i < m_iBuffLength; i++){
            cout << pRingBuffer[i] << " ";
        }
        cout << endl;
        for (int i = 0; i <= iReadIdx; i++){
            if (i == iReadIdx)
                cout << "r ";
            else
                cout << "  ";
        }
        cout << endl;
        for (int i = 0; i <= iWriteIdx; i++){
            if (i == iWriteIdx)
                cout << "w ";
            else
                cout << "  ";
        }
        cout << endl;
    }
private:
    CRingBuffer ();
    CRingBuffer(const CRingBuffer& that);

    int m_iBuffLength;              //!< length of the internal buffer
    T* pRingBuffer;
    int iReadIdx;
    int iWriteIdx;
    bool isFull;
};
#endif // __RingBuffer_hdr__
