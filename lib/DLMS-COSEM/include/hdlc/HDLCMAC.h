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

#include <atomic>
#include <cstdint>
#include <cstddef>
#include <string>
#include <queue>
#include <map>
#include <functional>

#include "ERROR_TYPE.h"
#include "ISimpleTimer.h"
#include "modcnt.h"
#include "packet.h"
#include "HDLCControl.h"
#include "HDLCAddress.h"
#include "HDLCOptions.h"
#include "HDLCStatistics.h"
#include "StateMachine.h"
#include "Callback.h"
#include "DLMSVector.h"

namespace EPRI
{
    class ISerialSocket;
    class DLDataRequestParameter;

    class HDLCMAC : public Callback<bool, uint16_t>, public StateMachine
	{
    	using PacketCallback = Callback<bool, HDLCControl::Control, Packet>;
    	
	public:
    	HDLCMAC() = delete;
    	//
    	// DL-INITIALIZE
    	//
    	HDLCMAC(const HDLCAddress& MyAddress, 
        	ISerialSocket * pSerial, 
        	const HDLCOptions& Options,
        	uint8_t MaxPreallocatedPacketBuffers);
    	virtual ~HDLCMAC();
    	
    	HDLCAddress MyAddress() const;
    	const HDLCStatistics& Statistics() const;
    	void ClearStatistics();
    	
    	virtual HDLCAddress ConnectedAddress() const;
    	virtual bool IsConnected() const;
    	//
    	// DL-SET_VALUE
    	//
    	virtual void SetOptions(const HDLCOptions& Options);
    	//
    	// DL-GET_VALUE
    	//
    	virtual HDLCOptions GetOptions() const;
        //
        // MA-DATA Service
        //
    	virtual bool DataRequest(const DLDataRequestParameter& Parameters);
    	
	protected:
        //
        // State Machine/DL-LM_EVENT
        //
    	enum States : uint8_t
    	{
        	ST_DISCONNECTED    = 0,
        	ST_IEC_CONNECT,
        	ST_CONNECTING_WAIT,
        	ST_CONNECTED,
        	ST_MAX_STATES
    	};

    	virtual void ST_Disconnected_Handler(EventData * pData) = 0;
    	virtual void ST_IEC_Connect_Handler(EventData * pData) = 0;
    	virtual void ST_Connecting_Wait_Handler(EventData * pData) = 0;
    	virtual void ST_Connected_Handler(EventData * pData);
    	//
    	// Packet Handlers
    	//
    	virtual bool I_Handler(const Packet& RXPacket);
    	
    	using PacketPtr = std::unique_ptr<Packet>;

    	virtual void Process() = 0;
    	
    	virtual HDLCErrorCode ProcessSerialReception(ERROR_TYPE Error, size_t BytesReceived);
    	Packet * GetWorkingRXPacket();
    	void ReleaseWorkingRXPacket();
    	void EnqueueWorkingRXPacket();            

    	virtual void ProcessSerialTransmission();
    	Packet * GetWorkingTXPacket();
    	void ReleaseWorkingTXPacket();
    	void EnqueueWorkingTXPacket();            
    	Packet * GetOutgoingPacket();
    	void ReleaseOutgoingPacket();
    	
    	virtual HDLCErrorCode ProcessPacketReception();
    	Packet * GetIncomingPacket();
    	void ReleaseIncomingPacket();
    	
    	void Serial_Connect(ERROR_TYPE Error);
        void Serial_Receive(ERROR_TYPE Error, size_t BytesReceived);
    	void Serial_Close(ERROR_TYPE Error);

    	bool ArmAsyncRead(uint32_t TimeOutInMs = 0, size_t MinimumSize = sizeof(uint8_t));
      	
        HDLCAddress							m_MyAddress;
    	HDLCAddress                         m_ConnectedAddress;
    	ISerialSocket *                     m_pSerial;
    	HDLCOptions                         m_CurrentOptions;
		std::shared_ptr<EPRI::ISimpleTimer>	m_pTimer;
		HDLCStatistics					    m_Statistics;
    	std::queue<PacketPtr>               m_Packets;
    	std::queue<PacketPtr>               m_RXPackets;
    	std::queue<PacketPtr>               m_TXPackets;
    	PacketCallback                      m_PacketCallback;
    	DLMSVector                          m_RXVector;
    	
	private:
    	void LockPackets();
    	void UnlockPackets();
    	
    	std::atomic_flag                    m_PacketLock = ATOMIC_FLAG_INIT;
    	PacketPtr                           m_pRXPacket = nullptr;
    	PacketPtr                           m_pTXPacket = nullptr;
	};
    //
    // Base Callback Parameters
    //
    struct HDLCCallbackParameter : public BaseCallbackParameter
    {
        HDLCCallbackParameter(const HDLCAddress& DA)
            : DestinationAddress(DA)
        {
        }
        HDLCAddress DestinationAddress;
    };
    //
    // IDENTIFY
    //
    struct DLIdentifyRequestParameter : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x100A;
        DLIdentifyRequestParameter(const HDLCAddress& DA)
            : HDLCCallbackParameter(DA)
        {
        }
    };
    struct DLIdentifyResponseParameter : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x100B;
        DLIdentifyResponseParameter(const HDLCAddress& DA, const uint8_t * pData)
            : HDLCCallbackParameter(DA)
        {
            SuccessCode = pData[0];
            ProtocolID = pData[1];
            ProtocolVersion = pData[2];
            ProtocolRevision = pData[3];
        }
        DLIdentifyResponseParameter(const HDLCAddress& DA)
            : HDLCCallbackParameter(DA)
        {
            SuccessCode = Packet::IDENTIFY_RESPONSE[0];
            ProtocolID = Packet::IDENTIFY_RESPONSE[1];
            ProtocolVersion = Packet::IDENTIFY_RESPONSE[2];
            ProtocolRevision = Packet::IDENTIFY_RESPONSE[3];
        }
        uint8_t SuccessCode;
        uint8_t ProtocolID;
        uint8_t ProtocolVersion;
        uint8_t ProtocolRevision;
    };
    //
    // CONNECT
    //
    struct DLConnectConfirmOrResponse : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x1001;
        DLConnectConfirmOrResponse(const HDLCAddress& DA)
            :
            HDLCCallbackParameter(DA)
        {
        }
        // Result
        // User_Information
    };
    
    struct DLConnectRequestOrIndication : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x1002;
        DLConnectRequestOrIndication(const HDLCAddress& DA)
            :
            HDLCCallbackParameter(DA)
        {
        }
        // User_Information
    };
    //
    // DATA
    //
    struct DLDataRequestParameter : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x1004;
        DLDataRequestParameter(const HDLCAddress& DA, HDLCControl::Control FT, const DLMSVector& D)
            :
            HDLCCallbackParameter(DA),
            FrameType(FT),
            Data(D)
        {
        }
        HDLCControl::Control FrameType;
        DLMSVector           Data;
    };
    //
    // DISCONNECT
    //
    struct DLDisconnectConfirmOrResponse : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x100A;
        DLDisconnectConfirmOrResponse(const HDLCAddress& DA)
            : HDLCCallbackParameter(DA)
        {
        }
        // Result
        // User_Information
    };
    
    struct DLDisconnectRequestOrIndication : public HDLCCallbackParameter
    {
        static const uint16_t ID = 0x100B;
        DLDisconnectRequestOrIndication(const HDLCAddress& DA)
            : HDLCCallbackParameter(DA)
        {
        }
        DLDisconnectRequestOrIndication()
            : HDLCCallbackParameter(HDLCAddress())
        {
        }

        // User_Information
    };
    template<typename TInternal>
    class MACEventData : public EventData
    {
    public:
        MACEventData(const TInternal& D)
            : Data(D)
        {
        }

        virtual void Release()
        {
            delete this;
        }
        
        TInternal Data;
        
    };
    
    using IdentifyEventData = MACEventData<DLIdentifyRequestParameter>;
    using IdentifyResponseData = MACEventData<DLIdentifyResponseParameter>;
    using ConnectEventData = MACEventData<DLConnectRequestOrIndication>;
    using ConnectResponseData = MACEventData<DLConnectConfirmOrResponse>;
    using PacketEventData = MACEventData<Packet>;
    using DataEventData = MACEventData<DLDataRequestParameter>;
    using DisconnectEventData = MACEventData<DLDisconnectRequestOrIndication>;
    using DisconnectResponseData = MACEventData<DLDisconnectConfirmOrResponse>;
    
    class HDLCClient : public HDLCMAC
    {
    public:
        HDLCClient() = delete;
    	//
    	// DL-INITIALIZE, DL-SETVALUE(HDLCOptions)
    	//
        HDLCClient(const HDLCAddress& MyAddress, 
            ISerialSocket * pSerial, 
            const HDLCOptions& Opt,
            uint8_t MaxPreallocatedPacketBuffers);
        virtual ~HDLCClient();
    	//
    	// IDENTIFY Service
    	//
        virtual bool IdentifyRequest(const DLIdentifyRequestParameter& Parameters);
        //
        // MA-CONNECT Service
        //
        virtual bool ConnectRequest(const DLConnectRequestOrIndication& Parameters);
        //
        // MA-DISCONNECT Service
        //
        virtual bool DisconnectRequest(const DLDisconnectRequestOrIndication& Parameters);
        
    protected:
        //
        // HDLCMAC
        //
        void Process();
  
    private:
        virtual void ST_Disconnected_Handler(EventData * pData) final;
        virtual void ST_IEC_Connect_Handler(EventData * pData) final;
        virtual void ST_Connecting_Wait_Handler(EventData * pData) final;
        virtual void ST_Connected_Handler(EventData * pData) final;
        //
        // Packet Handlers
        //
        bool UI_Handler(const Packet& RXPacket);
        bool UA_Handler(const Packet& RXPacket);
        bool DM_Handler(const Packet& RXPacket);
        bool IDENTR_Handler(const Packet& RXPacket);
      
    };
    
    class HDLCServer : public HDLCMAC
    {
    public:
        HDLCServer() = delete;
    	//
    	// DL-INITIALIZE, DL-SETVALUE(HDLCOptions)
    	//
        HDLCServer(const HDLCAddress& MyAddress, 
            ISerialSocket * pSerial, 
            const HDLCOptions& Opt,
            uint8_t MaxPreallocatedPacketBuffers);
        virtual ~HDLCServer();
        //
        // IDENTIFY Service
        //
        bool IdentifyResponse(const DLIdentifyResponseParameter& Parameters);
        //
        // MA-CONNECT Service
        //
        bool ConnectResponse(const DLConnectConfirmOrResponse& Parameters);
        //
        // MA-DISCONNECT Service
        //
        bool DisconnectResponse(const DLDisconnectConfirmOrResponse& Parameters);
        
    protected:
        //
        // HDLCMAC
        //
        void Process();
        
    private:
        virtual void ST_Disconnected_Handler(EventData * pData) final;
        virtual void ST_IEC_Connect_Handler(EventData * pData) final;
        virtual void ST_Connecting_Wait_Handler(EventData * pData) final;
        virtual void ST_Connected_Handler(EventData * pData) final;
        //
        // Packet Handlers
        //
        bool SNRM_Handler(const Packet& Packet);
        bool IDENT_Handler(const Packet& RXPacket);
        bool DISC_Handler(const Packet& RXPacket);
       
    };


} /* namespace EPRI */

