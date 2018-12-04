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

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "HDLCHelpers.h"
#include "COSEMAddress.h"

namespace EPRI
{
    class HDLCAddress
    {
    public:
        enum ShortReservedAddress : uint8_t
        {
            SHORT_NO_STATION = ReservedAddresses::NO_STATION,
            SHORT_MANAGEMENT = ReservedAddresses::MANAGEMENT,
            SHORT_PUBLIC_CLIENT = ReservedAddresses::PUBLIC_CLIENT,
            SHORT_BROADCAST = ReservedAddresses::BROADCAST,
            SHORT_CALLING = ReservedAddresses::CALLING
        };
        enum LongReservedAddress : uint16_t
        {
            LONG_NO_STATION    = ReservedAddresses::NO_STATION,
            LONG_MANAGEMENT    = ReservedAddresses::MANAGEMENT,
            LONG_BROADCAST     = 0x3FFF,
            LONG_CALLING       = 0x3FFE
        };
       
        HDLCAddress();
        HDLCAddress(uint8_t LogicalAddress);
        HDLCAddress(uint8_t LogicalAddressUpper, uint8_t PhysicalAddressLower);
        HDLCAddress(uint16_t LogicalAddressUpper, uint16_t PhysicalAddressLower);
        HDLCAddress(const HDLCAddress& Src);
        ~HDLCAddress();

        uint16_t LogicalAddress() const;
        uint16_t PhysicalAddress() const;
        void Copy(const HDLCAddress& Src);
        void Clear();

        HDLCAddress& operator=(const HDLCAddress& Src);
        bool operator ==(const HDLCAddress &b) const;
        HDLCAddress& Parse(const uint8_t * pAddressFromPHY);

        inline size_t Size() const
        {
            return m_Size;
        }
        inline bool IsEmpty() const
        {
            return 0 == m_Size;
        }
        inline const uint8_t * Get() const
        {
            return m_Address;
        }
        inline const uint8_t * operator&()
        {
            return m_Address;
        }
        
    protected:
        inline uint8_t GetAdjustedByte(int Index) const
        {
            return (m_Address[Index] & 0xFE) >> 1;
        }
        size_t  m_Size = 0;
        uint8_t m_Address[sizeof(uint16_t) + sizeof(uint16_t)] = { };
    };
    
}
