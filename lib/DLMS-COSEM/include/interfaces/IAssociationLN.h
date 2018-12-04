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

#include "ICOSEMInterface.h"

namespace EPRI
{
    const uint16_t CLSID_IAssociationLN = 15;
    
    class IAssociationLN_0 : public ICOSEMInterface
    {
        COSEM_DEFINE_SCHEMA(Object_List_Schema)
        COSEM_DEFINE_SCHEMA(Assoc_Partners_Schema)
        COSEM_DEFINE_SCHEMA(App_Context_Name_Schema)
        COSEM_DEFINE_SCHEMA(xDLMS_Schema)
        COSEM_DEFINE_SCHEMA(Mechanism_Schema)
        COSEM_DEFINE_SCHEMA(Assoc_Status_Schema)

    public :
        IAssociationLN_0();
        virtual ~IAssociationLN_0();
        
        enum Attributes : ObjectAttributeIdType
        {
            ATTR_OBJ_LIST            = 2,
            ATTR_PARTNERS_ID         = 3,
            ATTR_APP_CON_NAME        = 4,
            ATTR_XDLMS_CON_INFO      = 5,
            ATTR_AUTH_MECH_NAME      = 6,
            ATTR_SECRET              = 7,
            ATTR_STATUS              = 8,
            ATTR_SECURITY_SETUP_REF  = 9
        };
        
        enum AssociationState : uint8_t
        {
            non_associated      = 0,
            association_pending = 1,
            associated          = 2
        };
        
        COSEMAttribute<ATTR_OBJ_LIST, Object_List_Schema, 0x08>           object_list;
        COSEMAttribute<ATTR_PARTNERS_ID, Assoc_Partners_Schema, 0x10>     associated_partners_id;
        COSEMAttribute<ATTR_APP_CON_NAME, App_Context_Name_Schema, 0x18>  application_context_name;
        COSEMAttribute<ATTR_XDLMS_CON_INFO, xDLMS_Schema, 0x20>           xDLMS_context_type;
        COSEMAttribute<ATTR_AUTH_MECH_NAME, Mechanism_Schema, 0x28>       mechanism_name;
        COSEMAttribute<ATTR_SECRET, OctetStringSchema, 0x30>              secret;
        COSEMAttribute<ATTR_STATUS, Assoc_Status_Schema, 0x38>            association_status;
        COSEMAttribute<ATTR_SECURITY_SETUP_REF, OctetStringSchema, 0x40>  security_setup_reference;
        //
        // All Methods Are Optional; Not Implementing at this Time
        //
    };
    
    typedef IAssociationLN_0 IAssociationLN;
    
    class IAssociationLNObject : public IAssociationLN, public ICOSEMObject
    {
    public:
        IAssociationLNObject() = delete;
        IAssociationLNObject(const COSEMObjectInstanceCriteria& OIDCriteria, 
            uint16_t ShortNameBase = std::numeric_limits<uint16_t>::max());
        virtual ~IAssociationLNObject();
    };

}