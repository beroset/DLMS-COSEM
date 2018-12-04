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

#include "interfaces/IClock.h"

namespace EPRI
{
    COSEM_BEGIN_SCHEMA(IClock_0::Clock_Base_Schema)
        COSEM_ENUM_TYPE
        (   
            { 
                IClock_0::NOT_DEFINED, 
                IClock_0::INTERNAL_CRYSTAL, 
                IClock_0::MAINS_50HZ,
                IClock_0::MAINS_60HZ,
                IClock_0::GPS,
                IClock_0::RADIO
            }
        )
    COSEM_END_SCHEMA
        
    COSEM_BEGIN_SCHEMA(IClock_0::Adjusting_Time_Schema)
        COSEM_BEGIN_STRUCTURE
            COSEM_OCTET_STRING_TYPE
            COSEM_OCTET_STRING_TYPE
            COSEM_OCTET_STRING_TYPE
        COSEM_END_STRUCTURE
    COSEM_END_SCHEMA
    //
    // IClock_0
    //
    IClock_0::IClock_0()
        : ICOSEMInterface(CLSID_IClock, 0, 0, 1)
    {
        COSEM_BEGIN_ATTRIBUTES
            COSEM_ATTRIBUTE(time)
            COSEM_ATTRIBUTE(time_zone)
            COSEM_ATTRIBUTE(status)
            COSEM_ATTRIBUTE(daylight_savings_begin)
            COSEM_ATTRIBUTE(daylight_savings_end)
            COSEM_ATTRIBUTE(daylight_savings_deviation)
            COSEM_ATTRIBUTE(daylight_savings_enabled)
            COSEM_ATTRIBUTE(clock_base)
        COSEM_END_ATTRIBUTES
            
        COSEM_BEGIN_METHODS
            COSEM_METHOD(adjust_to_quarter)
            COSEM_METHOD(adjust_to_measuring_period)
            COSEM_METHOD(adjust_to_minute)
            COSEM_METHOD(adjust_to_preset_time)
            COSEM_METHOD(preset_adjusting_time)
            COSEM_METHOD(shift_time)
        COSEM_END_METHODS         
    }
        
    IClock_0::~IClock_0()
    {
    }
    //
    // IClockObject
    //
    IClockObject::IClockObject(const COSEMObjectInstanceCriteria& OIDCriteria, 
        uint16_t ShortNameBase /* = std::numeric_limits<uint16_t>::max() */)
        : ICOSEMObject(OIDCriteria, ShortNameBase)
    {
    }
        
    IClockObject::~IClockObject()
    {
    }
    
}