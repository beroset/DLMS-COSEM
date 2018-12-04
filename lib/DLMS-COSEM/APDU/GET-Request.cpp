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

#include "APDU/GET-Request.h"

namespace EPRI
{
        
    ASN_BEGIN_SCHEMA(Get_Request::Get_Request_Schema)
        ASN_BEGIN_CHOICE
            ASN_BEGIN_CHOICE_ENTRY_WITH_OPTIONS(Get_Request::Get_Request_Choice::get_request_normal, ASN::IMPLICIT)
                ASN_BEGIN_SEQUENCE
                    ASN_INVOKE_ID_AND_PRIORITY
                    ASN_BASE_TYPE(ASN::DT_Unsigned16)
                    ASN_FIXED_OCTET_STRING_TYPE(ASN::IMPLICIT, 6)
                    ASN_BASE_TYPE(ASN::DT_Integer8)
                    //
                    // TODO - Selective Access.  Phase II.
                    //
                    ASN_BASE_TYPE(ASN::DT_Unsigned8)
                ASN_END_SEQUENCE
            ASN_END_CHOICE_ENTRY
            //
            // TODO - Other Request Types. Phase II.
            //
        ASN_END_CHOICE
    ASN_END_SCHEMA
        
    Get_Request::Get_Request() :
       Get_Request_Base::APDUSingleType(Get_Request::Get_Request_Schema)
    {

    }
    
    Get_Request::~Get_Request()
    {
    }
    
    bool Get_Request::IsValid() const
    {
        return true;
    }
    //
    // Get_Request_Normal
    //
    Get_Request_Normal::Get_Request_Normal()
    {
    }
    
    Get_Request_Normal::~Get_Request_Normal()
    {
    }
        
    bool Get_Request_Normal::Parse(DLMSVector * pData,
        COSEMAddressType SourceAddress,
        COSEMAddressType DestinationAddress)
    {
        // Perform the base parse, which just loads
        // the stream.
        //
        if (Get_Request::Parse(pData, SourceAddress, DestinationAddress))
        {
            int8_t       Choice;
            DLMSValue    Value;

            m_Type.Rewind();
            if (ASNType::GetNextResult::VALUE_RETRIEVED == m_Type.GetNextValue(&Value) &&
                m_Type.GetChoice(&Choice) && 
                (Choice == get_request_normal))   
            {
                try
                {
                    if (IsSequence(Value))
                    {
                        DLMSSequence& Sequence = DLMSValueGetSequence(Value);
                        invoke_id_and_priority = Sequence[0].get<uint8_t>();
                        cosem_attribute_descriptor.class_id = Sequence[1].get<uint16_t>();
                        cosem_attribute_descriptor.instance_id = Sequence[2].get<DLMSVector>();
                        cosem_attribute_descriptor.attribute_id = Sequence[3].get<int8_t>();
                        return true;
                    }
                }
                catch (const std::exception&)
                {
                }
            }
        }
        return false;
    }
    
    std::vector<uint8_t> Get_Request_Normal::GetBytes()
    {
        m_Type.Clear();
        if (m_Type.SelectChoice(get_request_normal))
        {
            m_Type.Append(invoke_id_and_priority);
            m_Type.Append(cosem_attribute_descriptor.class_id);
            m_Type.Append(cosem_attribute_descriptor.instance_id);
            m_Type.Append(cosem_attribute_descriptor.attribute_id);
            //
            // TODO - Selective Access. Phase II.
            //
            m_Type.Append(uint8_t(0x00));
            return Get_Request::GetBytes();
        }
        return std::vector<uint8_t>();
    }


}