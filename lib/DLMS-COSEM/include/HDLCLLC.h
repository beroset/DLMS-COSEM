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

#include <functional>
#include "ISerial.h"
#include "hdlc/HDLCMAC.h"
#include "hdlc/HDLCOptions.h"
#include "Callback.h"
#include "Transport.h"

namespace EPRI
{
    class HDLCLLC : public Transport
	{
	public:
    	HDLCLLC() = delete;
    	//
    	// DL-INITIALIZE
    	//
    	HDLCLLC(HDLCMAC * pMAC);
    	virtual ~HDLCLLC();
    	
    	virtual ProcessResultType Process();

    	HDLCAddress MyAddress() const;
    	const HDLCStatistics& Statistics() const;
    	void ClearStatistics();
    	
    	virtual HDLCAddress ConnectedAddress() const;
    	virtual bool IsConnected() const;
    	//
        // DL-DATA Service
        //
    	virtual bool DataRequest(const DLDataRequestParameter& Parameters);
        //
        // Transport
        //
    	bool DataRequest(const DataRequestParameter& Parameters);
	
	protected:
        //
        // DL-CONNECT Service
        //
    	virtual bool MACConnectConfirmOrIndication(const BaseCallbackParameter& Paramters);
        //
        // DL-DATA Service
        //
    	virtual bool MACDataIndication(const BaseCallbackParameter& Parameters);
        //
        // DL-DISCONNECT Service
        //
    	virtual bool MACDisconnectConfirmOrIndication(const BaseCallbackParameter& Paramters);
        //
    	DLDataRequestParameter& AddLLCHeader(DLDataRequestParameter * pParameters);
    	
	private:
      	HDLCMAC *				 m_pMAC;
	};
    
    class HDLCClientLLC : public HDLCLLC
    {
    public:
    	//
    	// DL-INITIALIZE
    	//
        HDLCClientLLC(const HDLCAddress& MyAddress, 
            ISerialSocket * pSerial, 
            const HDLCOptions& Options,
            uint8_t MaxPreallocatedPacketBuffers = 10);
        virtual ~HDLCClientLLC();
    	//
    	// IDENTIFY Service
    	//
        virtual bool IdentifyRequest(const DLIdentifyRequestParameter& Parameters);
        void RegisterIdentifyConfirm(CallbackFunction Callback);
        //
        // DL-CONNECT Service
        //
        bool ConnectRequest(const DLConnectRequestOrIndication& Parameters);
        void RegisterConnectConfirm(CallbackFunction Callback);
        //
        // DL-DATA Service
        //
        void RegisterDataIndication(CallbackFunction Callback);
        //
        // DL-DISCONNECT Service
        //
        bool DisconnectRequest(const DLDisconnectRequestOrIndication& Parameters);
        void RegisterDisconnectConfirm(CallbackFunction Callback);

    protected:
        //
        // IDENTIFY Service
        //
        virtual bool MACIdentifyConfirm(const BaseCallbackParameter& Parameters);
        //
        // DL-CONNECT Service
        //
        virtual bool MACConnectConfirm(const BaseCallbackParameter& Paramters);
    	//
        // DL-DATA Service
        //
        virtual bool MACDataIndication(const BaseCallbackParameter& Parameters) final;
        //
        // DL-DISCONNECT Service
        //
        virtual bool MACDisconnectConfirm(const BaseCallbackParameter& Paramters);
        
    private:
        HDLCClient              m_MAC;
        
    };
    
    class HDLCServerLLC : public HDLCLLC
    {
    public:
    	//
    	// DL-INITIALIZE
    	//
        HDLCServerLLC(const HDLCAddress& MyAddress, 
            ISerialSocket * pSerial, 
            const HDLCOptions& Options,
            uint8_t MaxPreallocatedPacketBuffers = 10);
        virtual ~HDLCServerLLC();
        //
        // DL-CONNECT Service
        //
        bool ConnectResponse(const DLConnectConfirmOrResponse& Parameters);
        //
        // DL-DISCONNECT Service
        //
        bool DisconnectResponse(const DLDisconnectConfirmOrResponse& Parameters);
        
    protected:
        //
        // IDENTIFY Service (Handled Completely in the Server)
        //
        virtual bool IdentifyResponse(const DLIdentifyResponseParameter& Parameters);
        virtual bool MACIdentifyIndication(const BaseCallbackParameter& Parameters);
        //
        // DL-CONNECT Service
        //
        virtual bool OnConnectIndication(COSEMAddressType Address);
        virtual bool MACConnectIndication(const BaseCallbackParameter& Parameters);
        //
        // DL-DISCONNECT Service
        //
        virtual bool OnDisconnectIndication(COSEMAddressType Address);
        virtual bool MACDisconnectIndication(const BaseCallbackParameter& Parameters);
        
    private:
        HDLCServer              m_MAC;
    };

	
} /* namespace EPRI */
