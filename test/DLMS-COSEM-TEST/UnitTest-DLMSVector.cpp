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

#include <gtest/gtest.h>

#include "../../lib/DLMS-COSEM/DLMSVector.cpp"

using namespace EPRI;

TEST(DLMSVector, InitialSettings) 
{
    uint8_t u8;
    std::string String;
    
    DLMSVariant Variant1;
    DLMSVector Vector1;
    ASSERT_TRUE(Vector1.IsAtEnd());
    ASSERT_EQ(0, Vector1.Size());
    ASSERT_EQ(0, Vector1.GetReadPosition());
    ASSERT_FALSE(Vector1.SetReadPosition(1));
    ASSERT_FALSE(Vector1.Skip(1));

    ASSERT_FALSE(Vector1.Get<uint8_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<uint16_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<uint32_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<uint64_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<int8_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<int16_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<int32_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<int64_t>(&Variant1));
    ASSERT_FALSE(Vector1.Get<float>(&Variant1));
    ASSERT_FALSE(Vector1.Get<double>(&Variant1));
    ASSERT_FALSE(Vector1.GetBuffer(&u8, 1));
    ASSERT_FALSE(Vector1.Get(&String, 1));
    ASSERT_EQ(0, Vector1.GetBytes().size());
    ASSERT_EQ(0, Vector1.ToString().length());
}

TEST(DLMSVector, AppendGeneral)
{
    DLMSVector Vector1;
    
    ASSERT_EQ(0, Vector1.Size());
    ASSERT_EQ(0, Vector1.AppendExtra(10));
    ASSERT_EQ(10, Vector1.Size());
    Vector1[5] = 0x42;
    ASSERT_EQ(0x42, Vector1[5]);
    const std::vector<uint8_t> VALIDATE1 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00 };
    ASSERT_EQ(VALIDATE1, Vector1.GetBytes());
    ASSERT_TRUE(Vector1.Zero());
    const std::vector<uint8_t> VALIDATE2 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };    
    ASSERT_EQ(VALIDATE2, Vector1.GetBytes());
    Vector1.Clear();
    ASSERT_EQ(0, Vector1.Size());

}

TEST(DLMSVector, SkipsAndReads)
{
    DLMSVector Vector1({ 1, 2, 3, 4, 5, 6, 7 });
    ASSERT_EQ(7, Vector1.Size());
    ASSERT_FALSE(Vector1.IsAtEnd());
    ASSERT_TRUE(Vector1.Skip(3));
    ASSERT_EQ(4, Vector1.PeekByte());
    ASSERT_FALSE(Vector1.Skip(10));
    ASSERT_EQ(3, Vector1.GetReadPosition());
    ASSERT_EQ(4, Vector1.Get<uint8_t>());
    ASSERT_EQ(4, Vector1.GetReadPosition());
    uint8_t BUFFER[10] = { };
    const uint8_t COMPAREBUFFER[] = { 5, 6, 7 };
    ASSERT_TRUE(Vector1.PeekBuffer(BUFFER, 3));
    ASSERT_EQ(0, std::memcmp(BUFFER, COMPAREBUFFER, 3));
    ASSERT_EQ(4, Vector1.GetReadPosition());
    
}

TEST(DLMSVector, Operators)
{
    DLMSVector Vector1({ 1, 2, 3, 4 });
    DLMSVector Vector2(std::vector<uint8_t>({ 1, 2, 3, 4 }));   
    DLMSVector Vector3;
    ASSERT_EQ(Vector1, Vector2);
    
    Vector3 = Vector1;
    ASSERT_EQ(Vector1, Vector3);
}

TEST(DLMSVector, AppendBigEndian) 
{
    DLMSVariant Variant1;
    DLMSVector Vector1;
    
    Vector1.Append<uint8_t>(1);
    ASSERT_EQ(1, Vector1.Size());
    Vector1.Append<uint16_t>(0x1234);
    ASSERT_EQ(3, Vector1.Size());  
    Vector1.Append<uint32_t>(0x56789ABC);
    ASSERT_EQ(7, Vector1.Size());  
    Vector1.Append<uint64_t>(0xDEF0123456789ABC);
    ASSERT_EQ(15, Vector1.Size());  
    Vector1.Append<int8_t>(-1);
    ASSERT_EQ(16, Vector1.Size());  
    Vector1.Append<int16_t>(-1000);
    ASSERT_EQ(18, Vector1.Size());  
    Vector1.Append<int32_t>(-20000000);
    ASSERT_EQ(22, Vector1.Size());  
    Vector1.Append<int64_t>(-400000000000);
    ASSERT_EQ(30, Vector1.Size());  
    Vector1.AppendFloat(-42.1234);
    ASSERT_EQ(34, Vector1.Size());  
    Vector1.AppendDouble(-123456789.0123456789);
    ASSERT_EQ(42, Vector1.Size());  
    
    const uint8_t BYTES[] = { 0x87, 0x62, 0x10 };
    Vector1.AppendBuffer(BYTES, sizeof(BYTES));
    ASSERT_EQ(45, Vector1.Size());  
   
    ASSERT_EQ(0, Vector1.GetReadPosition());
    ASSERT_TRUE(Vector1.Get<uint8_t>(&Variant1));
    ASSERT_EQ(1, std::get<uint8_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint16_t>(&Variant1));
    ASSERT_EQ(0x1234, std::get<uint16_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint32_t>(&Variant1));
    ASSERT_EQ(0x56789ABC, std::get<uint32_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint64_t>(&Variant1));
    ASSERT_EQ(0xDEF0123456789ABC, std::get<uint64_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int8_t>(&Variant1));
    ASSERT_EQ(-1, std::get<int8_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int16_t>(&Variant1));
    ASSERT_EQ(-1000, std::get<int16_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int32_t>(&Variant1));
    ASSERT_EQ(-20000000, std::get<int32_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int64_t>(&Variant1));
    ASSERT_EQ(-400000000000, std::get<int64_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<float>(&Variant1));
    ASSERT_FLOAT_EQ(-42.1234, std::get<float>(Variant1));
    ASSERT_TRUE(Vector1.Get<double>(&Variant1));
    ASSERT_DOUBLE_EQ(-123456789.0123456789, std::get<double>(Variant1));
    
    uint8_t GETBYTES[3];
    ASSERT_TRUE(Vector1.GetBuffer(GETBYTES, sizeof(GETBYTES)));
    ASSERT_EQ(0, std::memcmp(GETBYTES, BYTES, sizeof(BYTES)));

}

TEST(DLMSVector, AppendLittleEndian) 
{
    
    DLMSVariant Variant1;
    DLMSVector Vector1;
    
    ASSERT_EQ(0, Vector1.Append<uint8_t>(1));
    ASSERT_EQ(1, Vector1.Size());
    ASSERT_EQ(1, Vector1.Append<uint16_t>(0x1234, false));
    ASSERT_EQ(3, Vector1.Size());  
    ASSERT_EQ(3, Vector1.Append<uint32_t>(0x56789ABC, false));
    ASSERT_EQ(7, Vector1.Size());  
    ASSERT_EQ(7, Vector1.Append<uint64_t>(0xDEF0123456789ABC, false));
    ASSERT_EQ(15, Vector1.Size());  
    ASSERT_EQ(15, Vector1.Append<int8_t>(-1));
    ASSERT_EQ(16, Vector1.Size());  
    ASSERT_EQ(16, Vector1.Append<int16_t>(-1000, false));
    ASSERT_EQ(18, Vector1.Size());  
    ASSERT_EQ(18, Vector1.Append<int32_t>(-20000000, false));
    ASSERT_EQ(22, Vector1.Size());  
    ASSERT_EQ(22, Vector1.Append<int64_t>(-400000000000, false));
    ASSERT_EQ(30, Vector1.Size());  
    ASSERT_EQ(30, Vector1.AppendFloat(-42.1234));
    ASSERT_EQ(34, Vector1.Size());  
    ASSERT_EQ(34, Vector1.AppendDouble(-123456789.0123456789));
    ASSERT_EQ(42, Vector1.Size());  
    
    const uint8_t BYTES[] = { 0x87, 0x62, 0x10 };
    ASSERT_EQ(42, Vector1.AppendBuffer(BYTES, sizeof(BYTES)));
    ASSERT_EQ(45, Vector1.Size());  
   
    ASSERT_EQ(0, Vector1.GetReadPosition());
    ASSERT_TRUE(Vector1.Get<uint8_t>(&Variant1));
    ASSERT_EQ(1, std::get<uint8_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint16_t>(&Variant1, false));
    ASSERT_EQ(0x1234, std::get<uint16_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint32_t>(&Variant1, false));
    ASSERT_EQ(0x56789ABC, std::get<uint32_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<uint64_t>(&Variant1, false));
    ASSERT_EQ(0xDEF0123456789ABC, std::get<uint64_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int8_t>(&Variant1));
    ASSERT_EQ(-1, std::get<int8_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int16_t>(&Variant1, false));
    ASSERT_EQ(-1000, std::get<int16_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int32_t>(&Variant1, false));
    ASSERT_EQ(-20000000, std::get<int32_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<int64_t>(&Variant1, false));
    ASSERT_EQ(-400000000000, std::get<int64_t>(Variant1));
    ASSERT_TRUE(Vector1.Get<float>(&Variant1));
    ASSERT_FLOAT_EQ(-42.1234, std::get<float>(Variant1));
    ASSERT_TRUE(Vector1.Get<double>(&Variant1));
    ASSERT_DOUBLE_EQ(-123456789.0123456789, std::get<double>(Variant1));
    
    uint8_t GETBYTES[3];
    ASSERT_TRUE(Vector1.GetBuffer(GETBYTES, sizeof(GETBYTES)));
    ASSERT_EQ(0, std::memcmp(GETBYTES, BYTES, sizeof(BYTES)));

}

TEST(DLMSVector, AppendVariant) 
{
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
    int8_t  i8 = -1;
    int16_t i16 = -32000;
    int32_t i32 = -34000000;
    int64_t i64 = -6500000000;
    float   f = -38738.23;
    double  d = -498459849894894.4784784748487;
    
    DLMSVariant Variant = 1;
    DLMSVector Vector1;
    
    ASSERT_EQ(0, Vector1.Append(Variant));
    Variant = 256;
    ASSERT_EQ(1, Vector1.Append(Variant));
    Variant = 65536;
    ASSERT_EQ(3, Vector1.Append(Variant));
    Variant = 0x1FFFFFFFF;
    ASSERT_EQ(7, Vector1.Append(Variant));
    Variant = i8;
    ASSERT_EQ(13, Vector1.Append(Variant));
   
}

TEST(DLMSVector, AppendString)
{
    DLMSVector Vector1;
    std::string String;
    
    ASSERT_EQ(0, Vector1.Append(std::string("1234567")));
    ASSERT_EQ(7, Vector1.Append(std::string("890")));
    ASSERT_TRUE(Vector1.Get(&String, 7));
    ASSERT_EQ(String, "1234567");
    ASSERT_TRUE(Vector1.Get(&String, 3));
    ASSERT_EQ(String, "890");
    ASSERT_FALSE(Vector1.Get(&String, 3));

    Vector1.Clear();
    ASSERT_EQ(0, Vector1.Append(std::string("123")));
    ASSERT_EQ(3, Vector1.Append(std::string("456")));
    
    std::string String1;
    ASSERT_TRUE(Vector1.Get(&String1, 2, true));
    ASSERT_EQ(String1, "12");
    ASSERT_TRUE(Vector1.Get(&String1, 4, true));
    ASSERT_EQ(String1, "123456");
    
}


