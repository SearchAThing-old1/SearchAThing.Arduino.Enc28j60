/*
* The MIT License(MIT)
* Copyright(c) 2016 Lorenzo Delana, https://searchathing.com
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

// Datasheet references
// --------------------
// # are relative to the DS39662E
// #E are relative to the DS80349C

#ifndef _SEARCHATHING_ARDUINO_ENC28J60_DRIVER_H
#define _SEARCHATHING_ARDUINO_ENC28J60_DRIVER_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <SearchAThing.Arduino.Utils\DebugMacros.h>

#include <SearchAThing.Arduino.Utils\Util.h>
#include <SearchAThing.Arduino.Utils\SList.h>
#include <SearchAThing.Arduino.Utils\RamData.h>
#include <SearchAThing.Arduino.Utils\IdStorage.h>
using namespace SearchAThing::Arduino;

#include <SearchAThing.Arduino.Net\Protocol.h>
#include <SearchAThing.Arduino.Net\ARP.h>
#include <SearchAThing.Arduino.Net\ICMP.h>
#include <SearchAThing.Arduino.Net\DNS.h>
#include <SearchAThing.Arduino.Net\DHCP.h>
#include <SearchAThing.Arduino.Net\EthDriver.h>
#include <SearchAThing.Arduino.Net\EthProcess.h>
using namespace SearchAThing::Arduino::Net;

#include "Registers.h"
#include "RxStatusVector.h"
#include "TxStatusVector.h"

#if USE_DHCP>0
#include <SearchAThing.Arduino.Net\DHCP.h>
#endif

//----------------------------------------------------------------------

// #4.1
#define DPIN_CS				10

// #3 - Ethernet Buffer (0x0000 -> 0x1FFF) = 8K
#define ETH_BUF_START	0x0000
#define ETH_BUF_END		0x1FFF

// maximize rx buffer cause mcu need to build each transmitted packets while
// the enc28j60 can receive more packets without blocking the mcu
// #E5

// TX(end)		: 0x1FFF = 8191
#define ETH_TX_END		ETH_BUF_END

// TX(begin)	: 0x1A12 = 6674
#define ETH_TX_BEGIN	(ETH_TX_END - MAX_FRAME_LENGTH + 1)

// RX(end)		: 0x1A11 = 6673
#define ETH_RX_END		(ETH_TX_BEGIN - 1)

// RX(start)	: 0x0000
#define ETH_RX_BEGIN	ETH_BUF_START

// Errata Silicon Revs
#define ETH_REV_B1	B0010
#define ETH_REV_B4	B0100
#define ETH_REV_B5	B0101
#define ETH_REV_B7	B0110

namespace SearchAThing
{

	namespace Arduino
	{

		namespace Enc28j60
		{

			class Driver : public EthDriver
			{

			private:
				byte currentBank = ETH_BANK0;
				bool currentBankUnset = true;

				LineStatusEnum lineStatus = LineStatusEnum::LinkDown;

				RxStatusVector rxStatusVector;
				TxStatusVector txStatusVector;

				uint16_t nextPktPtr;

				RamData macAddress;
				uint16_t lastPktCapacity;

				void InitSPI();
				void WaitAfterPoweron();
				void SetMacAddress(const RamData& _macAddress);
				void ResetRx();
				void ResetTx();
				void SetupRxMemoryBuffer();
				void SetupTxMemoryBuffer();
				void SetupMemoryBuffer();
				void SetupRxFilter();
				void DisableRx();
				void EnableRx();
				void DisableTxLoopback();
				void DumpRegs();

				// Set read pointer to given ptr
				void SetReadBufferMemoryPtr(uint16_t ptr);
				byte ReadBufferMemory();
				void ReadBufferMemory(byte *data, uint16_t len);
				void SetWriteBufferMemoryPtr(uint16_t ptr);
				void WriteBufferMemory(byte b);
				void WriteBufferMemory(const byte *data, uint16_t len);
				void SetBank(byte craddress);
				byte ReadControlRegister(byte craddress);
				void WriteControlRegister(byte craddress, byte data);
				void BitFieldSet(byte craddress, byte data);
				void BitFieldClear(byte craddress, byte data);
				void SoftReset();
				uint16_t FixRdPtr(uint16_t ptr);
				uint16_t WrapRxPtr(uint16_t ptr, uint16_t off);

				uint16_t PhyRead(byte praddress);
				void PhyWrite(byte praddress, uint16_t data);

				void ReadLinkStatus();
				void UpdateLineStatus();

				//--

				byte RevId();
				const RxStatusVector& GetRxStatusVector() const;
				void PrintRxStatusVector() const;

				const TxStatusVector& GetTxStatusVector() const;
				void PrintTxStatusVector() const;


			public:
				Driver();
				Driver(const RamData& _macAddress);

				// Destructor
				~Driver();

				const RamData& MacAddress() const;

				// determine the line status
				LineStatusEnum LineStatus();

				uint16_t Receive(byte *buf, uint16_t capacity);
				
				bool Transmit(const byte *buf, uint16_t len);
				
			};

		}

	}

}

#endif
