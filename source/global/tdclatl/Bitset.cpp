// Bitset.cpp : CBitset ‚ÌŽÀ‘•

#include "stdafx.h"
#include "Bitset.h"


// CBitset

STDMETHODIMP CBitset::get_Bit(short Index, VARIANT_BOOL* Value)
{
    *Value = m_bitset.get(Index);
    return S_OK;
}

STDMETHODIMP CBitset::put_Bit(short Index, VARIANT_BOOL Value)
{
    m_bitset.set(Index, (Value != 0));
    return S_OK;
}

STDMETHODIMP CBitset::Equals(IBitset* bitset, VARIANT_BOOL* retVal)
{
    CBitset* b = dynamic_cast<CBitset*>(bitset);
    if (!b) return Error("Invalid param.", IID_IBitset);
    *retVal = (m_bitset ==  b->m_bitset);   
    return S_OK;
}

STDMETHODIMP CBitset::Contains(IBitset* bitset, VARIANT_BOOL all, VARIANT_BOOL* retVal)
{
    CBitset* b = dynamic_cast<CBitset*>(bitset);
    if (!b) return Error("Invalid param bitset.", IID_IBitset);
    *retVal = m_bitset.contains(b->m_bitset, (all != 0));   
    return S_OK;
}


    
