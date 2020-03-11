
#include "Vector.h"
#include "Util.h"

#include "Dtw.h"

CDtw::CDtw( void )
{
    reset();
}

CDtw::~CDtw( void )
{
    for (int i = 0; i < m_iNumRows; i++)
        delete [] m_ppfCostMatrix[i];
    delete [] m_ppfCostMatrix;

    for (int i = 0; i < m_iNumRows; i++)
        delete [] m_ppiOptDirection[i];
    delete [] m_ppiOptDirection;
    
    delete [] m_ppiPathResult[0];
    delete [] m_ppiPathResult[1];
    delete [] m_ppiPathResult;
}

Error_t CDtw::init( int iNumRows, int iNumCols )
{
    reset();

    if (iNumRows <= 0 || iNumCols <= 0)
        return kFunctionInvalidArgsError;

    m_iNumRows = iNumRows;
    m_iNumCols = iNumCols;
    
    m_ppfCostMatrix = new float*[iNumRows];
    for (int i = 0; i < iNumRows; i++)
        m_ppfCostMatrix[i] = new float[iNumCols];

    m_ppiOptDirection = new int*[iNumRows];
    for (int i = 0; i < iNumRows; i++)
        m_ppiOptDirection[i] = new int[iNumCols];
    
    m_ppiPathResult = new int*[2];
    m_ppiPathResult[0] = new int[m_iNumRows+m_iNumCols-2];
    m_ppiPathResult[1] = new int[m_iNumRows+m_iNumCols-2];

    m_bInit = true;

    return kNoError;
}

Error_t CDtw::reset()
{
    m_bInit = false;
    m_iNumRows = 0;
    m_iNumCols = 0;
    m_iPathLength = 0;
    return kNoError;
}

Error_t CDtw::process(float **ppfDistanceMatrix)
{
    if (m_bInit == false)
        return kNotInitializedError;

    if (ppfDistanceMatrix == 0)
        return kFunctionInvalidArgsError;

    // init
    m_ppfCostMatrix[0][0] = ppfDistanceMatrix[0][0];
    m_ppiOptDirection[0][0] = -1;
    for (int i = 1; i < m_iNumRows; i++)
    {
        m_ppfCostMatrix[i][0] = m_ppfCostMatrix[i-1][0] + ppfDistanceMatrix[i][0];
        m_ppiOptDirection[i][0] = kVert;
    }
    for (int j = 1; j < m_iNumCols; j++)
    {
        m_ppfCostMatrix[0][j] = m_ppfCostMatrix[0][j-1] + ppfDistanceMatrix[0][j];
        m_ppiOptDirection[0][j] = kHoriz;
    }

    float directionCost[3] = {0,0,0};

    // recursion
    for (int i = 1; i < m_iNumRows; i++)
        for (int j = 1; j < m_iNumCols; j++)
        {
            directionCost[kHoriz] = m_ppfCostMatrix[i][j-1];
            directionCost[kVert] = m_ppfCostMatrix[i-1][j];
            directionCost[kDiag] = m_ppfCostMatrix[i-1][j-1];


            if (directionCost[kHoriz] <= directionCost[kVert] && directionCost[kHoriz] <= directionCost[kDiag])
            {
                m_ppfCostMatrix[i][j] = directionCost[kHoriz] + ppfDistanceMatrix[i][j];
                m_ppiOptDirection[i][j] = kHoriz;

            }
            else if (directionCost[kVert] <= directionCost[kHoriz] && directionCost[kVert] <= directionCost[kDiag])
            {
                m_ppfCostMatrix[i][j] = directionCost[kVert] + ppfDistanceMatrix[i][j];
                m_ppiOptDirection[i][j] = kVert;
            }
            else
            {
                m_ppfCostMatrix[i][j] = directionCost[kDiag] + ppfDistanceMatrix[i][j];
                m_ppiOptDirection[i][j] = kDiag;
            }
        }

    // track back
    m_iPathLength = 1;
    int curRow = m_iNumRows-1;
    int curCol = m_iNumCols-1;
    while (m_ppiOptDirection[curRow][curCol] != -1)
    {
        switch(m_ppiOptDirection[curRow][curCol])
        {
            case kHoriz:    curCol -= 1;    break;
            case kVert:     curRow -= 1;    break;
            case kDiag:     curCol -= 1;    curRow -= 1;    break;
            default:        break;
        }
        m_ppiPathResult[0][m_iPathLength-1] = curRow;
        m_ppiPathResult[1][m_iPathLength-1] = curCol;
        m_iPathLength++;
    }

    return kNoError;
}

int CDtw::getPathLength()
{   
    return m_iPathLength;
}

float CDtw::getPathCost() const
{
    return m_ppfCostMatrix[m_iNumRows-1][m_iNumCols-1];
}

Error_t CDtw::getPath( int **ppiPathResult ) const
{
    for (int i = 0; i < m_iPathLength-1; i++)
    {
        ppiPathResult[0][i] = m_ppiPathResult[0][m_iPathLength-2 - i];
        ppiPathResult[1][i] = m_ppiPathResult[1][m_iPathLength-2 - i];
    }
    ppiPathResult[0][m_iPathLength-1] = m_iNumRows-1;
    ppiPathResult[1][m_iPathLength-1] = m_iNumCols-1;
    return kNoError;
}

