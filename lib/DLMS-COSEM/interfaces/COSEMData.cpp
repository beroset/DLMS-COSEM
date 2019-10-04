// ===========================================================================
// Copyright (c) 2018, Electric Power Research Institute (EPRI)
// All rights reserved.
//
// DLMS-COSEM ("this software") is licensed under BSD 3-Clause license.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// *  Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// *  Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// *  Neither the name of EPRI nor the names of its contributors may
//    be used to endorse or promote products derived from this software without
//    specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.
//
// This EPRI software incorporates work covered by the following copyright and permission
// notices. You may not use these works except in compliance with their respective
// licenses, which are provided below.
//
// These works are provided by the copyright holders and contributors "as is" and any express or
// implied warranties, including, but not limited to, the implied warranties of merchantability
// and fitness for a particular purpose are disclaimed.
//
// This software relies on the following libraries and licenses:
//
// ###########################################################################
// Boost Software License, Version 1.0
// ###########################################################################
//
// * asio v1.10.8 (https://sourceforge.net/projects/asio/files/)
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 

#include <algorithm>
#include <set>
#include <stdexcept>

#include "APDU/ASNType.h"
#include "interfaces/COSEMData.h"

namespace EPRI
{
    COSEM_BEGIN_SCHEMA(OctetStringSchema)
        COSEM_OCTET_STRING_TYPE
    COSEM_END_SCHEMA
    COSEM_BEGIN_SCHEMA(LongSchema)
        COSEM_LONG_TYPE
    COSEM_END_SCHEMA
    COSEM_BEGIN_SCHEMA(UnsignedSchema)
        COSEM_UNSIGNED_TYPE
    COSEM_END_SCHEMA
    COSEM_BEGIN_SCHEMA(IntegerSchema)
        COSEM_INTEGER_TYPE
    COSEM_END_SCHEMA
    COSEM_BEGIN_SCHEMA(BooleanSchema)
        COSEM_BOOLEAN_TYPE
    COSEM_END_SCHEMA
        
    //
    // COSEMType
    //
    COSEMType::COSEMType()
    {
    }

    COSEMType::COSEMType(SchemaEntryPtr Schema)
        : m_pSchema(Schema)
        , m_pCurrentSchema(Schema)
    {
    }
    
    COSEMType::COSEMType(SchemaBaseType DT)
        : m_pSchema(m_SingleDataType)
        , m_pCurrentSchema(m_SingleDataType)
    {
        m_SingleDataType[0].m_SchemaType = DT;
    }
    
    COSEMType::COSEMType(SchemaBaseType DT, const DLMSVariant& Value)
        : m_pSchema(m_SingleDataType)
        , m_pCurrentSchema(m_SingleDataType)
    {
        m_SingleDataType[0].m_SchemaType = DT;
        InternalAppend(Value);
        Rewind();
    }

    COSEMType::~COSEMType()
    {
    }
    
    void COSEMType::GetBytes(DLMSVector * pDestination) const
    {
        pDestination->Append(m_Data.GetBytes());
    }
    
    std::vector<uint8_t> COSEMType::GetBytes() const
    {
        return m_Data.GetBytes();
    }
        
    bool COSEMType::IsEmpty() const
    {
        return m_Data.Size() == 0;
    }

    void COSEMType::Clear()
    {
        m_Data.Clear();
        Rewind();
    }
    
    void COSEMType::Rewind()
    {
        m_pCurrentSchema = m_pSchema;
        m_Data.SetReadPosition(0);
        while (m_GetStates.size())
        {
            m_GetStates.pop();
        }
        while (m_AppendStates.size())
        {
            m_AppendStates.pop();
        }
    }
    
    bool COSEMType::Parse(DLMSVector * pData)
    {
        return m_Data.Append(pData) >= 0;
    }

    bool COSEMType::Parse(const DLMSVector& Data)
    {
        return m_Data.Append(Data) >= 0;
    }
    
#define CURRENT_APPEND_STATE m_AppendStates.top()
    bool COSEMType::SelectChoice(COSEMDataType Choice)
    {
        //
        // PRECONDITIONS
        //
        if (m_AppendStates.empty())
        {
            m_AppendStates.emplace(nullptr, ST_SIMPLE, Choice);
            return true;
        }
        //
        // The only time we can select a choice is if we are sitting within
        // a CHOICE section.
        //
        if (nullptr != CURRENT_APPEND_STATE.m_SchemaEntry)
        {
            CURRENT_APPEND_STATE.m_Choice = Choice;
            return true;
        }
        return false;
    }

#define CURRENT_GET_STATE m_GetStates.top()
    bool COSEMType::GetChoice(COSEMDataType * pChoice)
    {
        if (m_GetStates.size() && INVALID_CHOICE != CURRENT_GET_STATE.m_Choice)
        {
            *pChoice = CURRENT_GET_STATE.m_Choice;
            return true;
        }
        return false;

    }
    
    COSEMType::GetNextResult COSEMType::InternalSimpleGet(SchemaEntryPtr SchemaEntry, DLMSVariant * pValue)
    {
        GetNextResult RetVal = INVALID_CONDITION;
        COSEMDataType DT = COSEMDataType(m_Data.Get<uint8_t>());
        if (DT != COSEM_SCHEMA_DATA_TYPE(SchemaEntry))
        {
            return INVALID_STREAM;
        }
        switch (DT)
        {
        case NULL_DATA:
            return VALUE_RETRIEVED;
        case BOOLEAN:    
            RetVal = m_Data.Get<bool>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case BIT_STRING:     
            RetVal = COSEMBitString::Get(SchemaEntry, this, pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case DOUBLE_LONG: 
            RetVal = m_Data.Get<int32_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case DOUBLE_LONG_UNSIGNED:
            RetVal = m_Data.Get<uint32_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case OCTET_STRING:   
            {
                size_t     Length = 0;
                DLMSVector Vector;
                if (ASNType::GetLength(&m_Data, &Length) &&
                    m_Data.GetVector(&Vector, Length))
                {
                    *pValue = Vector;
                    RetVal = VALUE_RETRIEVED;
                }
                else
                {
                    RetVal = INVALID_CONDITION;
                }
            }
            break;
        case VISIBLE_STRING: 
            {
                size_t      Length = 0;
                std::string String;
                if (ASNType::GetLength(&m_Data, &Length) &&
                    m_Data.Get(&String, Length))
                {
                    *pValue = String;
                    RetVal = VALUE_RETRIEVED;
                }
                else
                {
                    RetVal = INVALID_CONDITION;
                }
            }
            break;
        case UTF8_STRING:    

        case INTEGER:        
            RetVal = m_Data.Get<int8_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case ENUM:    
        case UNSIGNED:       
            RetVal = m_Data.Get<uint8_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case LONG:           
            RetVal = m_Data.Get<int16_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case LONG_UNSIGNED:  
            RetVal = m_Data.Get<uint16_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case LONG64:         
            RetVal = m_Data.Get<int64_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case LONG64_UNSIGNED:
            RetVal = m_Data.Get<uint64_t>(pValue) ? VALUE_RETRIEVED : INVALID_CONDITION;
            break;
        case BCD:            
        case FLOATING_POINT: 
        case FLOAT32:        
        case FLOAT64:        
        case DATE_TIME:      
        case DATE:           
        case TIME:           
        case DONT_CARE:      
        default:
            throw std::out_of_range("InternalSimpleGet: Type not implemented.");
        }
        return RetVal;
    }
    
    COSEMType::GetNextResult COSEMType::GetNextValue(DLMSValue * pValue)
    {
        GetNextResult       GetNextRetVal = INVALID_CONDITION;
        DLMSSequence        Sequence;
        //
        // PRECONDITIONS
        //
        if (m_Data.Size() == 0)
        {
            return VALUE_EMPTY;
        }
        if (m_Data.IsAtEnd())
        {
            return END_OF_SCHEMA;
        }
        if (m_GetStates.empty())
        {
            m_GetStates.emplace(nullptr, ST_SIMPLE, INVALID_CHOICE);
        }
        while (VALUE_RETRIEVED == (GetNextRetVal = GetNextSchemaEntry(&CURRENT_GET_STATE.m_SchemaEntry)))
        {
            switch (CURRENT_GET_STATE.m_State)
            {
            case ST_SIMPLE:
                if (BEGIN_CHOICE_T == 
                    COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    m_GetStates.emplace(nullptr, ST_CHOICE, INVALID_CHOICE);
                    break;
                }
                else if (COSEM_IS_STRUCTURE_BEGIN(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    m_GetStates.emplace(nullptr, ST_STRUCTURE, INVALID_CHOICE);
                    break;
                }
                else 
                {
                    DLMSVariant Value;
                    GetNextRetVal = InternalSimpleGet(CURRENT_GET_STATE.m_SchemaEntry, &Value);
                    if (VALUE_RETRIEVED == GetNextRetVal)
                    {
                        *pValue = Value;
                    }
                    return GetNextRetVal;
                }
                return SCHEMA_MISMATCH;
            case ST_CHOICE:
                if (END_CHOICE_T ==  
                    COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    m_GetStates.pop();
                }
                else if (COSEMDataType(m_Data.PeekByte()) == COSEM_SCHEMA_DATA_TYPE(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    CURRENT_GET_STATE.m_Choice = COSEM_SCHEMA_DATA_TYPE(CURRENT_GET_STATE.m_SchemaEntry);
                    if (COSEM_IS_STRUCTURE_BEGIN(CURRENT_GET_STATE.m_SchemaEntry))
                    {
                        m_GetStates.emplace(nullptr, ST_STRUCTURE, CURRENT_GET_STATE.m_Choice);
                        break;
                    }
                    else
                    {
                        DLMSVariant Value;
                        GetNextRetVal = InternalSimpleGet(CURRENT_GET_STATE.m_SchemaEntry, &Value);
                        if (VALUE_RETRIEVED == GetNextRetVal)
                        {
                            *pValue = Value;
                        }
                        return GetNextRetVal;
                    }
                }
                break;
            case ST_STRUCTURE:
                if (COSEM_IS_STRUCTURE_END(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    m_GetStates.pop();
                    *pValue = Sequence;
                    if (Sequence.size())
                    {
                        return VALUE_RETRIEVED; 
                    }
                    return NO_VALUE_FOUND;
                }
                else if (BEGIN_CHOICE_T == COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_GET_STATE.m_SchemaEntry))
                {
                    m_GetStates.emplace(nullptr, ST_CHOICE, INVALID_CHOICE);
                    break;
                }
                else
                {
                    //
                    // PRECONDITIONS
                    //
                    if (-1 == CURRENT_GET_STATE.m_SequenceIndex)
                    {
                        size_t ExpectedLength = 0;
                        if (m_Data.Get<uint8_t>() == COSEMDataType::STRUCTURE &&
                            ASNType::GetLength(&m_Data, &ExpectedLength) &&
                            ExpectedLength == StructureElementCount(CURRENT_GET_STATE.m_SchemaEntry))
                        {
                            CURRENT_GET_STATE.m_SequenceIndex = 0;
                        }
                        else
                        {
                            return SCHEMA_MISMATCH;
                        }
                    }
                        
                    DLMSVariant Value;
                    GetNextRetVal = InternalSimpleGet(CURRENT_GET_STATE.m_SchemaEntry, &Value);
                    if (VALUE_RETRIEVED == GetNextRetVal)
                    {
                        Sequence.push_back(Value);
                        break;
                    }
                }
                return SCHEMA_MISMATCH;
            default:
                throw std::out_of_range("GetNextValue Not implemented.");
            }
        }
        return GetNextRetVal;
    }
    
    bool COSEMType::Append(const DLMSValue& Value)
    {
        return InternalAppend(Value);
    }
    
    bool COSEMType::Append()
    {
        //
        // NULL-DATA Case
        //
        return InternalAppend(DLMSVariant(blank()));
    }
    //
    // Operators
    //
    bool COSEMType::operator==(const std::vector<uint8_t>& rhs) const
    {
        return rhs == m_Data.GetBytes();
    }
    
    bool COSEMType::operator==(const COSEMType& rhs) const
    {
        return rhs.m_Data == m_Data;
    }
    
    COSEMType& COSEMType::operator=(const DLMSVector& rhs)
    {
        m_Data = rhs;
        return *this;
    }

    COSEMType::operator const DLMSVector&()
    {
        return m_Data;
    }
    
    //
    // Protected Methods
    //
    void COSEMType::SetSchemaType(SchemaBaseType DT)
    {
        m_pSchema = m_SingleDataType;
        m_SingleDataType[0].m_SchemaType = DT;
        Clear();
    }
    
    COSEMType::GetNextResult COSEMType::GetNextSchemaEntry(SchemaEntryPtr * ppSchemaEntry)
    {
        *ppSchemaEntry = m_pCurrentSchema;
        if (nullptr == m_pCurrentSchema)
        {
            return INVALID_CONDITION;
        }
        
        if (!COSEM_IS_SCHEMA_END(m_pCurrentSchema))
        {
            ++m_pCurrentSchema;
            return VALUE_RETRIEVED;
        }
        return END_OF_SCHEMA;
    }

    class COSEMTypeAppendVisitor 
    {
    public:
        COSEMTypeAppendVisitor() = delete;
        COSEMTypeAppendVisitor(SchemaEntryPtr SchemaEntry, DLMSVector * pVector)
            : m_SchemaEntry(SchemaEntry), m_pVector(pVector)
        {
        }
        
        void operator()(const blank& Value)
        {
        }
        void operator()(const bool& Value)
        {
            IntegralAppend(Value ? 1 : 0);
        }
        void operator()(const int8_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const uint8_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const int16_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const uint16_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const int32_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const uint32_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const int64_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const uint64_t& Value)
        {
            IntegralAppend(Value);
        }
        void operator()(const std::string& Value)
        {
            switch (COSEM_SCHEMA_DATA_TYPE(m_SchemaEntry))
            {
            case NULL_DATA:
                break;
            case VISIBLE_STRING:
                ASNType::AppendLength(Value.length(), m_pVector);
                m_pVector->Append(Value);
                break;
            default:
                throw std::logic_error("Not implemented");
            }
        }
        void operator()(const double& Value)
        {
            FloatingPointAppend(Value);
        }
        void operator()(const DLMSVector& Value)
        {
            switch (COSEM_SCHEMA_DATA_TYPE(m_SchemaEntry))
            {
            case NULL_DATA:
                break;
            case BIT_STRING:
                m_pVector->Append(Value);
                break;
            case OCTET_STRING:
                {
                    size_t Length = Value.Size();
                    ASNType::AppendLength(Length, m_pVector);
                    m_pVector->Append(Value, 0, Length);
                }
                break;                
            default:
                throw std::logic_error("Not implemented");
            }
        }
        void operator()(const std::vector<uint32_t>& Value)
        {
            throw std::logic_error("Not implemented");
        }
        void operator()(const DLMSBitSet& Value)
        {
            switch (COSEM_SCHEMA_DATA_TYPE(m_SchemaEntry))
            {
            case NULL_DATA:
                break;
            case BIT_STRING:
                {
                    COSEMBitString Conversion(COSEM_SCHEMA_DATA_TYPE_SIZE(m_SchemaEntry), 
                                             Value);
                    Conversion.GetBytes(m_pVector);
                }
                break;
            default:
                throw std::logic_error("Not implemented");
            }
        }
        
    protected:
        void IntegralAppend(uint64_t VectorValue)
        {
            switch (COSEM_SCHEMA_DATA_TYPE(m_SchemaEntry))
            {
            case NULL_DATA:
                break;
            case UNSIGNED:
            case BOOLEAN:
                m_pVector->Append<uint8_t>(VectorValue);
                break;
            case DOUBLE_LONG:
                m_pVector->Append<int32_t>(VectorValue);
                break;
            case DOUBLE_LONG_UNSIGNED:
                m_pVector->Append<uint32_t>(VectorValue);
                break;
            case INTEGER:
                m_pVector->Append<int8_t>(VectorValue);
                break;
            case LONG:
                m_pVector->Append<int16_t>(VectorValue);
                break;
            case LONG_UNSIGNED:
                m_pVector->Append<uint16_t>(VectorValue);
                break;
            case LONG64:
                m_pVector->Append<int64_t>(VectorValue);
                break;
            case LONG64_UNSIGNED:
                m_pVector->Append<uint64_t>(VectorValue);
                break;
            default:
                throw std::logic_error("Not implemented");
            }
        }
        
        void FloatingPointAppend(double VectorValue)
        {
            switch (COSEM_SCHEMA_DATA_TYPE(m_SchemaEntry))
            {
            case NULL_DATA:
                break;
            //        case FLOAT32:
            //        case FLOAT64:
            default:
                throw std::logic_error("Not implemented");
            }
        }
        
        DLMSVector *   m_pVector;
        SchemaEntryPtr m_SchemaEntry;
    };

    bool COSEMType::InternalSimpleAppend(SchemaEntryPtr SchemaEntry, const DLMSVariant& Value)
    {
        m_Data.Append(uint8_t(COSEM_SCHEMA_DATA_TYPE(SchemaEntry)));
        try
        {
            COSEMTypeAppendVisitor Visitor(SchemaEntry, &m_Data);
            apply_visitor(Visitor, Value);
        }
        catch (const std::exception&)
        {
            return false;
        }
        return true;
    }
    
    ssize_t COSEMType::StructureElementCount(SchemaEntryPtr pSchemaEntry) const
    {
        ssize_t        ElementCount = 0;
        size_t         Level = 0;
        SchemaEntryPtr pEntry = ++pSchemaEntry;
            
        while (!COSEM_IS_SCHEMA_END(pEntry))
        {
            if (COSEM_IS_BEGINNING(pEntry))
            {
                ++Level;
            }
            else if (COSEM_IS_ENDING(pEntry))
            {
                --Level;
            }
            if (0 == Level)
            {
                if (COSEM_IS_STRUCTURE_END(pEntry))
                {
                    return ElementCount;
                }
                ++ElementCount;
            }
            ++pEntry;
        }
        return -1;
    }
    
    bool COSEMType::InternalAppend(const DLMSValue& Value)
    {
        if (m_AppendStates.empty())
        {
            m_AppendStates.emplace(nullptr, ST_SIMPLE, INVALID_CHOICE);
        }
        while (VALUE_RETRIEVED == GetNextSchemaEntry(&CURRENT_APPEND_STATE.m_SchemaEntry))
        {
            switch (CURRENT_APPEND_STATE.m_State)
            {
            case ST_SIMPLE:
                if (BEGIN_CHOICE_T == 
                    COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    m_AppendStates.emplace(nullptr, ST_CHOICE, CURRENT_APPEND_STATE.m_Choice);
                    break;
                }
                else if (COSEM_IS_STRUCTURE_BEGIN(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    m_AppendStates.emplace(nullptr, ST_STRUCTURE, CURRENT_APPEND_STATE.m_Choice);
                    break;
                }
                else 
                {
                    return InternalSimpleAppend(CURRENT_APPEND_STATE.m_SchemaEntry, Value.get<DLMSVariant>());                  
                }
                return false;
            case ST_CHOICE:
                if (END_CHOICE_T ==  
                    COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    m_AppendStates.pop();
                }
                else if (CURRENT_APPEND_STATE.m_Choice == COSEM_SCHEMA_DATA_TYPE(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    if ((COSEMDataType::STRUCTURE == CURRENT_APPEND_STATE.m_Choice) &&
                        COSEM_IS_STRUCTURE_BEGIN(CURRENT_APPEND_STATE.m_SchemaEntry))
                    {
                        m_AppendStates.emplace(nullptr, ST_STRUCTURE, CURRENT_APPEND_STATE.m_Choice);
                        break;
                    }
                    else
                    {
                        return InternalSimpleAppend(CURRENT_APPEND_STATE.m_SchemaEntry, Value.get<DLMSVariant>());
                    }
                }
                break;
            case ST_STRUCTURE:
                if (COSEM_IS_STRUCTURE_END(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    m_AppendStates.pop();
                    return true;
                }
                else if (BEGIN_CHOICE_T == COSEM_SCHEMA_INTERNAL_DATA_TYPE(CURRENT_APPEND_STATE.m_SchemaEntry))
                {
                    m_AppendStates.emplace(nullptr, ST_CHOICE, CURRENT_APPEND_STATE.m_Choice);
                    break;
                }
                else
                {
                    if (IsSequence(Value))
                    {
                        const DLMSSequence& Sequence = DLMSValueGetSequence(Value);
                        //
                        // PRECONDITIONS
                        //
                        if (-1 == CURRENT_APPEND_STATE.m_SequenceIndex)
                        {
                            if (Sequence.size() == StructureElementCount(CURRENT_APPEND_STATE.m_SchemaEntry))
                            {
                                m_Data.Append<uint8_t>(COSEMDataType::STRUCTURE);
                                ASNType::AppendLength(Sequence.size(), &m_Data);
                                CURRENT_APPEND_STATE.m_SequenceIndex = 0;
                            }
                            else
                            {
                                return false;
                            }
                        }
                        if (false == InternalSimpleAppend(CURRENT_APPEND_STATE.m_SchemaEntry, Sequence[CURRENT_APPEND_STATE.m_SequenceIndex++]))
                        {
                            return false;
                        }
                        if (CURRENT_APPEND_STATE.m_SequenceIndex == Sequence.size())
                        {
                            return true;
                        }
                        break;
                    }
                    //
                    // ONLY Sequence Allowed for STRUCTURE
                    //
                }
                return false;
                
            default:
                return false;
            }
        }
        return false;
    }

    bool COSEMType::InternalAppend(const DLMSVector& Value)
    {
        return m_Data.Append(Value) >= 0;
    }
    //
    // COSEMBitString
    //
    COSEMBitString::COSEMBitString()
        : COSEMType(BIT_STRING)
    {
    }
        
    COSEMBitString::~COSEMBitString()
    {
    }
        
    COSEMBitString::COSEMBitString(size_t BitsExpected)
        : COSEMType(BIT_STRING), m_BitsExpected(BitsExpected)
    {
        if (0 == BitsExpected)
        {
            throw std::invalid_argument("BitsExpected must be > 0");
        }
    }
    
    size_t COSEMBitString::GetBitLength() const
    {
        return m_BitsExpected;
    }

    COSEMBitString::COSEMBitString(size_t BitsExpected, const DLMSBitSet& Value)
        : COSEMType(BIT_STRING), m_BitsExpected(BitsExpected)
    {
        if (0 == BitsExpected)
        {
            throw std::invalid_argument("BitsExpected must be > 0");
        }
        uint8_t ByteLength = BitsExpected / 8;
        uint8_t ValueBitIndex = 0;
        if (BitsExpected % 8)
        {
            ++ByteLength;
        }
        ASNType::AppendLength(BitsExpected, &m_Data);
        size_t Offset = m_Data.AppendExtra(ByteLength);
        for (uint8_t ByteIndex = Offset + ByteLength - 1; 
             ByteIndex >= Offset; --ByteIndex)
        {
            uint8_t CurrentByte = 0;
            for (int BitShift = 0; BitShift < 8; BitShift++)
            {
                CurrentByte |= (Value[ValueBitIndex++] << BitShift);
            }
            m_Data[ByteIndex] = CurrentByte;
        }
        
    }
    
    COSEMBitString::operator DLMSVariant() const
    {
        return m_Data;
    }
    
    bool COSEMBitString::Peek(SchemaEntryPtr SchemaEntry, const COSEMType& Value, DLMSVariant * pVariant, size_t * pBytes /* = nullptr */)
    {
        DLMSVector Output;
        size_t     Offset = Value.m_Data.GetReadPosition();
        size_t     Length = 0;
        size_t     LengthBytes = 0;
        
        if (ASNType::PeekLength(Value.m_Data, 0, &Length, &LengthBytes))
        {
            Offset += LengthBytes;
            LengthBytes += (Length / 8);
            if (Length % 8)
            {
                ++LengthBytes;
            }
        }
        else
        {
            return false;
        }
        if (LengthBytes > Value.m_Data.Size())
        {
            return false;
        }
        if (pBytes)
        {
            *pBytes = LengthBytes;
        }
        DLMSBitSet Bitset;
        for (ssize_t BitIndex = Length - 1; BitIndex >=0; --BitIndex)
        {
            Bitset[BitIndex] = Value.m_Data[Offset] & 
                                    (1 << (BitIndex % 8));
            if (BitIndex % 8 == 0)
            {
                ++Offset;
            }
        }
        *pVariant = Bitset;
        return true;
    }
    
    bool COSEMBitString::Get(SchemaEntryPtr SchemaEntry, COSEMType * pValue, DLMSVariant * pVariant)
    {
        size_t Bytes = 0;
        if (Peek(SchemaEntry, *pValue, pVariant, &Bytes))
        {
            pValue->m_Data.Skip(Bytes);
            return true;
        }
        return false;
        
    }

}
