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

#include <deque>
#include <vector>
#include <execinfo.h>

#include "ERROR_TYPE.h"
#include "ISerial.h"

using namespace EPRI;

class DummySerial : public EPRI::ISerialSocket
{
public:
    DummySerial()
    {
    }
    //
    // ISerialSocket
    //
    EPRI::ERROR_TYPE Open(const char * DestinationAddress = nullptr, int Port = DEFAULT_DLMS_PORT)
    {
        return EPRI::SUCCESSFUL;
    }
    ConnectCallbackFunction RegisterConnectHandler(ConnectCallbackFunction Callback)
    {
        ConnectCallbackFunction RetVal = m_Connect;
        m_Connect = Callback;
        return RetVal;
    }
    EPRI::ERROR_TYPE SetOptions(const EPRI::ISerial::Options& /*Opt*/)
    {
        return EPRI::SUCCESSFUL;
    }
    EPRI::ERROR_TYPE Write(const DLMSVector& Data, bool Asynchronous = false)
    {
        int Who = WhoCalled();
        for (int Index = 0; Index < Data.Size(); ++Index)
        {
            m_SerialStream[Who].push_back(Data[Index]);
        }
        return EPRI::SUCCESSFUL;
    }
    WriteCallbackFunction RegisterWriteHandler(WriteCallbackFunction Callback)
    {
        WriteCallbackFunction RetVal = m_Write;
        m_Write = Callback;
        return RetVal;
    }
    ERROR_TYPE Read(DLMSVector * pData,
        size_t ReadAtLeast = 0,
        uint32_t TimeOutInMS = 0,
        size_t * pActualBytes = nullptr)
    {
        int Who = WhoCalled() ^ 1;
        if (m_SerialStream[Who].size())
        {
            if (ReadAtLeast >= m_SerialStream[Who].size() || ReadAtLeast == 0)
            {
                ReadAtLeast = m_SerialStream[Who].size();
            }
            if (pData)
            {
                while (ReadAtLeast--)
                {
                    pData->Append<uint8_t>(m_SerialStream[Who].front());
                    m_SerialStream[Who].pop_front();
                }
            }
            else
            {
                while (ReadAtLeast--)
                {
                    m_AsyncBuffer[Who].Append<uint8_t>(m_SerialStream[Who].front());
                    m_SerialStream[Who].pop_front();
                }                    
            }
            return EPRI::SUCCESSFUL;
        }
        return ~EPRI::SUCCESSFUL;
    }
    bool AppendAsyncReadResult(DLMSVector * pData, size_t ReadAtLeast = 0)
    {
        int Who = WhoCalled() ^ 1;
        if (ReadAtLeast >= m_AsyncBuffer[Who].Size() || ReadAtLeast == 0)
        {
            ReadAtLeast = m_AsyncBuffer[Who].Size();
        }
        return pData->Append(m_AsyncBuffer[Who], 0, ReadAtLeast);
    }
    ReadCallbackFunction RegisterReadHandler(ReadCallbackFunction Callback)
    {
        ReadCallbackFunction RetVal = m_Read;
        m_Read = Callback;
        return RetVal;
    }
    EPRI::ERROR_TYPE Close()
    {
        return EPRI::SUCCESSFUL;
    }
    virtual CloseCallbackFunction RegisterCloseHandler(CloseCallbackFunction Callback)
    {
        CloseCallbackFunction RetVal = m_Close;
        m_Close = Callback;
        return RetVal;
    }
    EPRI::ERROR_TYPE Flush(FlushDirection Direction)
    {
        m_SerialStream[WhoCalled()].clear();
        return EPRI::SUCCESSFUL;
    }
    bool IsConnected()
    {
        return true;
    }
    
    bool IsWriteAsExpected(const std::vector<uint8_t>& Expected)
    {
        int Who = WhoCalled();
        if (m_SerialStream[Who].size() == Expected.size())
            return std::equal(m_SerialStream[Who].begin(), m_SerialStream[Who].end(), Expected.begin());
        return false;
    }
    
protected:
    int WhoCalled()
    {
        void * AddressList[64];
        int    AddressLength = backtrace(AddressList, sizeof(AddressList) / sizeof(void*));
        if (AddressLength)
        {
            char ** Symbols = backtrace_symbols(AddressList, AddressLength);
            for (int Index = 1; Index < AddressLength; Index++)
            {
                std::string Line(Symbols[Index]);
                if (Line.find("HDLCClient") != std::string::npos)
                    return 0;
                else if (Line.find("HDLCServer") != std::string::npos)
                    return 1;
            }
        }
        return 0;
    }
    
private:
    std::deque<uint8_t>     m_SerialStream[2];
    ConnectCallbackFunction m_Connect;
    WriteCallbackFunction   m_Write;
    ReadCallbackFunction    m_Read;
    CloseCallbackFunction   m_Close;
    DLMSVector              m_AsyncBuffer[2];

};
