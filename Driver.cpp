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

#include <SearchAThing.Arduino.Utils\DebugMacros.h>

#include "Driver.h"

#include <MemoryFree/MemoryFree.h>
#include <SPI.h>

#include <SearchAThing.Arduino.Utils\Util.h>
#include <SearchAThing.Arduino.Utils\SList.h>
#include <SearchAThing.Arduino.Utils\RamData.h>
using namespace SearchAThing::Arduino;

#include <SearchAThing.Arduino.Net\Protocol.h>
#include <SearchAThing.Arduino.Net\ARP.h>
#include <SearchAThing.Arduino.Net\Checksum.h>
#include <SearchAThing.Arduino.Net\ICMP.h>
#include <SearchAThing.Arduino.Net\DHCP.h>
using namespace SearchAThing::Arduino::Net;

SPISettings SPI_SETTINGS(20e6, MSBFIRST, SPI_MODE0);

#define SPI_BEGIN()	{ SPI.beginTransaction(SPI_SETTINGS); digitalWrite(DPIN_CS, LOW); }
#define SPI_END()	{ digitalWrite(DPIN_CS, HIGH); SPI.endTransaction(); }

namespace SearchAThing
{

	namespace Arduino
	{

		namespace Enc28j60
		{

			// #4.2.1
			void Driver::InitSPI()
			{
				pinMode(DPIN_CS, OUTPUT);
				SPI.begin();
			}

			// #2.2
			void Driver::WaitAfterPoweron()
			{
				// #E2
				delay(1);

				//while (ReadControlRegister(ETH_ESTAT) & ETH_ESTAT_CLKRDY);
			}

			// #6.5 [fullduplex mode]
			void Driver::SetMacAddress(const RamData& _macAddress)
			{
				macAddress = _macAddress;

				// #6.5.1
				BitFieldSet(ETH_MACON1, ETH_MACON1_TXPAUS | ETH_MACON1_RXPAUS | ETH_MACON1_MARXEN);

				// #6.5.2
				BitFieldSet(ETH_MACON3, ETH_MACON3_PADCFG0 | ETH_MACON3_TXCRCEN | ETH_MACON3_FRMLNEN | ETH_MACON3_FULDPX);
				// #6.5.4			
				WriteControlRegister(ETH_MAMXFL, lowByte(MAX_FRAME_LENGTH));
				WriteControlRegister(ETH_MAMXFH, highByte(MAX_FRAME_LENGTH));
				// #6.5.5
				WriteControlRegister(ETH_MABBIPG, 0x15); // Full-Duplex
				// #6.5.6
				WriteControlRegister(ETH_MAIPGL, 0x12);
				// #6.5.7
				WriteControlRegister(ETH_MAIPGH, 0x0C);
				// #6.5.9
				WriteControlRegister(ETH_MAADR1, macAddress.Buf()[0]);
				WriteControlRegister(ETH_MAADR2, macAddress.Buf()[1]);
				WriteControlRegister(ETH_MAADR3, macAddress.Buf()[2]);
				WriteControlRegister(ETH_MAADR4, macAddress.Buf()[3]);
				WriteControlRegister(ETH_MAADR5, macAddress.Buf()[4]);
				WriteControlRegister(ETH_MAADR6, macAddress.Buf()[5]);
			}

			void Driver::ResetRx()
			{
#if defined DEBUG && defined DEBUG_ETH_RX
				DPrint(F("* Reset rx")); DNewline();
#endif

				DisableRx();

#if defined DEBUG && defined DEBUG_ASSERT
				if (ReadControlRegister(ETH_EPKTCNT) != 0)
				{
					DPrint(F("* expected pktcnt=0")); DNewline();
				}
#endif		
				while (ReadControlRegister(ETH_EPKTCNT) > 0) BitFieldSet(ETH_ECON2, ETH_ECON2_PKTDEC);

				// #11.4
				BitFieldSet(ETH_ECON1, ETH_ECON1_RXRST);
				BitFieldClear(ETH_ECON1, ETH_ECON1_RXRST);

				SetupRxMemoryBuffer();

				EnableRx();
			}

			void Driver::ResetTx()
			{
#if defined DEBUG && defined DEBUG_ETH_TX
				DPrint(F("* Reset tx")); DNewline();
#endif

				// #11.3
				BitFieldSet(ETH_ECON1, ETH_ECON1_TXRST);
				BitFieldClear(ETH_ECON1, ETH_ECON1_TXRST);

				SetupTxMemoryBuffer();
			}

			// #3.2.1 - Set RX start/end/ptr
			void Driver::SetupRxMemoryBuffer()
			{
				nextPktPtr = ETH_RX_BEGIN; // 0

				// RX start
				WriteControlRegister(ETH_ERXSTL, lowByte(ETH_RX_BEGIN));
				WriteControlRegister(ETH_ERXSTH, highByte(ETH_RX_BEGIN));

				// #E14
				auto _rdPtr = FixRdPtr(nextPktPtr);

				// RX ptr
				WriteControlRegister(ETH_ERXRDPTL, lowByte(_rdPtr));
				WriteControlRegister(ETH_ERXRDPTH, highByte(_rdPtr));

				// RX end
				WriteControlRegister(ETH_ERXNDL, lowByte(ETH_RX_END));
				WriteControlRegister(ETH_ERXNDH, highByte(ETH_RX_END));
			}

			// #3.2.2 - Set TX start/end
			void Driver::SetupTxMemoryBuffer()
			{
				// TX start
				WriteControlRegister(ETH_ETXSTL, lowByte(ETH_TX_BEGIN));
				WriteControlRegister(ETH_ETXSTH, highByte(ETH_TX_BEGIN));

				// TX end
				WriteControlRegister(ETH_ETXNDL, lowByte(ETH_TX_END));
				WriteControlRegister(ETH_ETXNDH, highByte(ETH_TX_END));
			}

			// #3.2 - Set RX start/end/ptr and TX start/end
			void Driver::SetupMemoryBuffer()
			{
				SetupRxMemoryBuffer();
				SetupTxMemoryBuffer();
			}

			// #8-3
			typedef struct PatternMatchFilter
			{
				byte DstAddr1 : 1; // 0
				byte DstAddr2 : 1;
				byte DstAddr3 : 1;
				byte DstAddr4 : 1;
				byte DstAddr5 : 1;
				byte DstAddr6 : 1;

				byte SrcAddr1 : 1;
				byte SrcAddr2 : 1; // 7
				byte SrcAddr3 : 1; // 8
				byte SrcAddr4 : 1;
				byte SrcAddr5 : 1;
				byte SrcAddr6 : 1;

				byte Type1 : 1;
				byte Type2 : 1;
				byte _pad : 2; // 15-14				
			};

			// #8
			void Driver::SetupRxFilter()
			{
				WriteControlRegister(ETH_ERXFCON,
					// invalid CRC packets will be discarded
					ETH_ERXFCON_CRCEN |

					// #8.1 - accepts MAC address packets
					ETH_ERXFCON_UCEN |

					// accepts pattern-match packets
					ETH_ERXFCON_PMEN);

				// #8.2
				{
					PatternMatchFilter filterPattern;

					byte *b = (byte *)&filterPattern;
					memset(b, 0, sizeof(PatternMatchFilter));

					// set the filter to check agains dstmac and ethertype ( srcmac can be any )

					filterPattern.DstAddr1 = filterPattern.DstAddr2 = filterPattern.DstAddr3 =
						filterPattern.DstAddr4 = filterPattern.DstAddr5 = filterPattern.DstAddr6 = 1;

					filterPattern.Type1 = filterPattern.Type2 = 1;

					WriteControlRegister(ETH_EPMM0, b[0]);
					WriteControlRegister(ETH_EPMM1, b[1]);
				}

				{
					byte filterData[6 + 6 + 2];
					memset(filterData, 0xff, 6); // set srcmac data
					memset(filterData + 6, 0x00, 6);
					BufWrite16(filterData + 12, Eth2Type::Eth2Type_ARP); // set ethertype data

					auto chksum = CheckSum(filterData, sizeof(filterData));
					WriteControlRegister(ETH_EPMCSL, lowByte(chksum));
					WriteControlRegister(ETH_EPMCSH, highByte(chksum));
				}
			}

			// #7.2.1
			void Driver::DisableRx()
			{
				BitFieldClear(ETH_ECON1, ETH_ECON1_RXEN);
			}

			// #7.2.1
			void Driver::EnableRx()
			{
				BitFieldClear(ETH_EIR, ETH_EIR_RXERIF);
				BitFieldSet(ETH_ECON1, ETH_ECON1_RXEN);
			}

			// #6.6
			void Driver::DisableTxLoopback()
			{
				PhyWrite(ETH_PHCON2, ETH_PHCON2_HDLDIS);
			}

			void Driver::DumpRegs()
			{
				DNewline();

				// ECON1				
				{
					DPrint(F("\t[ECON1]\t"));
					auto econ1 = ReadControlRegister(ETH_ECON1);
					DPrint(F("txr:")); DPrintBool(econ1 & ETH_ECON1_TXRST);
					DPrint(F(" rxr:")); DPrintBool(econ1 & ETH_ECON1_RXRST);
					DPrint(F(" dma:")); DPrintBool(econ1 & ETH_ECON1_DMAST);
					DPrint(F(" chk:")); DPrintBool(econ1 & ETH_ECON1_CSUMEN);
					DPrint(F(" txs:")); DPrintBool(econ1 & ETH_ECON1_TXRTS);
					DPrint(F(" rxn:")); DPrintBool(econ1 & ETH_ECON1_RXEN);
					DPrint(F(" bs1:")); DPrintBool(econ1 & ETH_ECON1_BSEL1);
					DPrint(F(" bs0:")); DPrintBool(econ1 & ETH_ECON1_BSEL0);
					DNewline();
				}

				// ECON2
				{

					DPrint(F("\t[ECON2]\t"));
					auto econ2 = ReadControlRegister(ETH_ECON2);
					DPrint(F("ain:")); DPrintBool(econ2 & ETH_ECON2_AUTOINC);
					DPrint(F(" pkd:")); DPrintBool(econ2 & ETH_ECON2_PKTDEC);
					DPrint(F(" pwr:")); DPrintBool(econ2 & ETH_ECON2_PWRSV);
					DPrint(F(" vpr")); DPrintBool(econ2 & ETH_ECON2_VRPS);
					DNewline();
				}

				// ESTAT
				{
					DPrint(F("\t[ESTAT]\t"));
					auto estat = ReadControlRegister(ETH_ESTAT);
					DPrint(F("int:")); DPrintBool(estat & ETH_ESTAT_INT);
					DPrint(F(" bfe:")); DPrintBool(estat & ETH_ESTAT_BUFER);
					DPrint(F(" ltc:")); DPrintBool(estat & ETH_ESTAT_LATECOL);
					DPrint(F(" rxb:")); DPrintBool(estat & ETH_ESTAT_RXBUSY);
					DPrint(F(" txa:")); DPrintBool(estat & ETH_ESTAT_TXABRT);
					DPrint(F(" ckr:")); DPrintBool(estat & ETH_ESTAT_CLKRDY);
					DNewline();
				}

				// EIE
				{
					DPrint(F("\t[EIE]\t"));
					auto eie = ReadControlRegister(ETH_EIE);
					DPrint(F("int:")); DPrintBool(eie & ETH_EIE_INTIE);
					DPrint(F(" pkt:")); DPrintBool(eie & ETH_EIE_PKTIE);
					DPrint(F(" dma:")); DPrintBool(eie & ETH_EIE_DMAIE);
					DPrint(F(" lnk:")); DPrintBool(eie & ETH_EIE_LINKIE);
					DPrint(F(" txe:")); DPrintBool(eie & ETH_EIE_TXERIE);
					DPrint(F(" rxe:")); DPrintBool(eie & ETH_EIE_RXERIE);
					DNewline();
				}

				// EIR
				{
					DPrint(F("\t[EIR]\t"));
					auto eir = ReadControlRegister(ETH_EIR);
					DPrint(F("pkt:")); DPrintBool(eir & ETH_EIR_PKTIF);
					DPrint(F(" dma:")); DPrintBool(eir & ETH_EIR_DMAIF);
					DPrint(F(" lnk:")); DPrintBool(eir & ETH_EIR_LINKIF);
					DPrint(F(" txi:")); DPrintBool(eir & ETH_EIR_TXIF);
					DPrint(F(" txe:")); DPrintBool(eir & ETH_EIR_TXERIF);
					DPrint(F(" rxe:")); DPrintBool(eir & ETH_EIR_RXERIF);
					DNewline();
				}

			}

			// #4.2.2
			void Driver::SetReadBufferMemoryPtr(uint16_t ptr)
			{
				WriteControlRegister(ETH_ERDPTL, lowByte(ptr));
				WriteControlRegister(ETH_ERDPTH, highByte(ptr));
			}

			// #4.2.4
			void Driver::SetWriteBufferMemoryPtr(uint16_t ptr)
			{
				WriteControlRegister(ETH_EWRPTL, lowByte(ptr));
				WriteControlRegister(ETH_EWRPTH, highByte(ptr));
			}

			// #4.2.2 - Read single byte from buffer memory at ERDPT
			byte Driver::ReadBufferMemory()
			{
				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_RBM);
				auto data = SPI.transfer(0);
				SPI_END();

				return data;
			}

			// #4.2.2 - Read len bytes starting from ERDPT
			void Driver::ReadBufferMemory(byte *data, uint16_t len)
			{
				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_RBM);
				while (len)
				{
					*data = SPI.transfer(ETH_SPIOP_RBM);
					++data;
					--len;
				}
				SPI_END();
			}

			// #4.2.4
			void Driver::WriteBufferMemory(byte b)
			{
				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_WBM);
				SPI.transfer(b);
				SPI_END();
			}

			// #4.2.4
			void Driver::WriteBufferMemory(const byte *data, uint16_t len)
			{
				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_WBM);
				while (len)
				{
					SPI.transfer(*data);
					++data;
					--len;
				}
				SPI_END();
			}

			// #3.1.1 - Set bank from Compact Register Address
			void Driver::SetBank(byte craddress)
			{
				auto reg = ETH_CRA_REG(craddress);

				if (reg >= ETH_EIE && reg <= ETH_ECON1) return; // all banks register

				byte bank = (craddress >> 5) & B11;
				if (currentBank != bank || currentBankUnset)
				{
					BitFieldClear(ETH_ECON1, ETH_ECON1_BSEL1 | ETH_ECON1_BSEL0);
					BitFieldSet(ETH_ECON1, bank);

					currentBank = bank;
					currentBankUnset = false;
				}
			}

			// #4.2.1
			byte Driver::ReadControlRegister(byte craddress)
			{
				SetBank(craddress);

				// #4-3

				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_RCR | ETH_CRA_REG(craddress));

				if (craddress & ETH_MAC_MII_FLAG) SPI.transfer(0); // #4-4		

				auto res = SPI.transfer(0);
				SPI_END();

				return res;
			}

			// #4.2.3
			void Driver::WriteControlRegister(byte craddress, byte data)
			{
				SetBank(craddress);

				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_WCR | ETH_CRA_REG(craddress));
				SPI.transfer(data);
				SPI_END();
			}

			// #4.2
			void Driver::BitFieldSet(byte craddress, byte data)
			{
				SetBank(craddress);

				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_BFS | ETH_CRA_REG(craddress));
				SPI.transfer(data);
				SPI_END();
			}

			// #4.2
			void Driver::BitFieldClear(byte craddress, byte data)
			{
				SetBank(craddress);

				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_BFC | ETH_CRA_REG(craddress));
				SPI.transfer(data);
				SPI_END();
			}

			// #4.2
			void Driver::SoftReset()
			{
				SPI_BEGIN();
				SPI.transfer(ETH_SPIOP_SRC);
				SPI_END();

				delay(1); // #E2
			}

			// #E14
			uint16_t Driver::FixRdPtr(uint16_t ptr)
			{
#if defined DEBUG && defined DEBUG_ASSERT
				if (ptr % 2 != 0)
				{
					DPrint(F("* expected an even ptr: ")); DPrintHex(ptr); DNewline();
					return ptr;
				}
#endif
				if (ptr - 1 > ETH_RX_END)
					return ETH_RX_END;
				else
					return ptr - 1; // odd value
			}

			// #7-1
			uint16_t Driver::WrapRxPtr(uint16_t ptr, uint16_t off)
			{
				if (ptr + off > ETH_RX_END)
					return ptr + off - (ETH_RX_END - ETH_RX_BEGIN + 1);
				else
					return ptr + off;
			}

			// #3.3.1
			uint16_t Driver::PhyRead(byte praddress)
			{
				// #3.3.1.1
				WriteControlRegister(ETH_MIREGADR, praddress);

				// #3.3.1.2
				BitFieldSet(ETH_MICMD, ETH_MICMD_MIIRD);

				// #3.3.1.3

				while (ReadControlRegister(ETH_MISTAT) & ETH_MISTAT_BUSY)
				{
					delayMicroseconds(11); // 10.24 us
				}

				// #3.3.1.4
				BitFieldClear(ETH_MICMD, ETH_MICMD_MIIRD);

				auto high = ReadControlRegister(ETH_MIRDH);
				auto low = ReadControlRegister(ETH_MIRDL);

				return (((uint16_t)high) << 8) | low;
			}

			// #3.3.2
			void Driver::PhyWrite(byte praddress, uint16_t data)
			{
				// #3.3.2.1
				WriteControlRegister(ETH_MIREGADR, praddress);

				// #3.3.2.2
				WriteControlRegister(ETH_MIWRL, lowByte(data));

				// #3.3.2.3
				WriteControlRegister(ETH_MIWRH, highByte(data));

				delayMicroseconds(11); // 10.24 us
				while (ReadControlRegister(ETH_MISTAT) & ETH_MISTAT_BUSY);
			}

			// #3.3.4
			void Driver::ReadLinkStatus()
			{
				auto res = PhyRead(ETH_PHSTAT2);

				auto prev = lineStatus;

				if ((res & ETH_PHSTAT2_LSTAT) != 0)
					lineStatus = LineStatusEnum::LinkUp;
				else
					lineStatus = LineStatusEnum::LinkDown;

#if defined DEBUG && defined DEBUG_ETH_DRIVER
				if (prev != lineStatus)
				{
					DPrint(F("LINK STATUS CHANGED:")); DPrint(lineStatus == LineStatusEnum::LinkUp); DNewline();
				}
#endif
			}

			void Driver::UpdateLineStatus()
			{
				//lineStatus = LineStatusEnum::LinkUp;

				ReadLinkStatus();

				auto eir = ReadControlRegister(ETH_EIR);

				if (eir & ETH_EIR_DMAIF)
				{
#if defined DEBUG && defined DEBUG_ETH_DRIVER
					DPrint(F("*DMA")); DNewline();
#endif
					// #12.1.6
					BitFieldClear(ETH_EIR, ETH_EIR_DMAIF);
				}

				if (eir & ETH_EIR_LINKIF)
				{
					ReadLinkStatus();

					// #12.1.5
					PhyRead(ETH_PHIR);
				}
			}

			//--

			// #3.3.5
			byte Driver::RevId()
			{
				return ReadControlRegister(ETH_EREVID) & B11111;
			}				

			const RxStatusVector& Driver::GetRxStatusVector() const { return rxStatusVector; }

			void Driver::PrintRxStatusVector() const
			{
				DPrint(F("cnt:")); DPrint(rxStatusVector.receivedByteCount);
				DPrint(F(" evd:")); DPrintBool(rxStatusVector.longEvtDropEvt);
				DPrint(F(" ces:")); DPrintBool(rxStatusVector.carrierEventSeen);
				DPrint(F(" cre:")); DPrintBool(rxStatusVector.crcError);
				DPrint(F(" lce:")); DPrintBool(rxStatusVector.lengthCheckError);
				DPrint(F(" lor:")); DPrintBool(rxStatusVector.lengthOutOfRange);
				DPrint(F(" rok:")); DPrintBool(rxStatusVector.receivedOk);
				DPrint(F(" mcs:")); DPrintBool(rxStatusVector.receivedMulticast);
				DPrint(F(" brc:")); DPrintBool(rxStatusVector.receivedBroadcast);
				DPrint(F(" drn:")); DPrintBool(rxStatusVector.dribbleNibble);
				DPrint(F(" ctf:")); DPrintBool(rxStatusVector.receiveControlFrame);
				DPrint(F(" pct:")); DPrintBool(rxStatusVector.receivePauseControlFrame);
				DPrint(F(" unk:")); DPrintBool(rxStatusVector.receiveUnknownOpcode);
				DPrint(F(" vla:")); DPrintBool(rxStatusVector.receiveVLAN);
				DPrint(F(" pkm:")); DPrint(lastPktCapacity);
				DPrint(F(" mfb:")); DPrint(FreeMemoryMaxBlock());
				DPrint(F(" mfs:")); DPrint(FreeMemorySum());

				DNewline();
			}

			const TxStatusVector& Driver::GetTxStatusVector() const { return txStatusVector; }

			void Driver::PrintTxStatusVector() const
			{
				DPrint(F(" cnt:")); DPrint(txStatusVector.txdByteCount);
				DPrint(F(" col:")); DPrint(txStatusVector.txCollCount);
				DPrint(F(" cre:")); DPrintBool(txStatusVector.txCrcErr);
				DPrint(F(" lor:")); DPrintBool(txStatusVector.txLengthORange);
				DPrint(F(" dne:")); DPrintBool(txStatusVector.txDone);
				DPrint(F(" mcs:")); DPrintBool(txStatusVector.txMcast);
				DPrint(F(" bcs:")); DPrintBool(txStatusVector.txBroadCast);
				DPrint(F(" dfr:")); DPrintBool(txStatusVector.txPktDefer);
				DPrint(F(" edr:")); DPrintBool(txStatusVector.txExcDefer);
				DPrint(F(" ecl:")); DPrintBool(txStatusVector.txExcCollision);
				DPrint(F(" lcl:")); DPrintBool(txStatusVector.txLateColl);
				DPrint(F(" gia:")); DPrintBool(txStatusVector.txGiant);
				DPrint(F(" und:")); DPrintBool(txStatusVector.txUnderrun);
				DPrint(F(" wcn:")); DPrint(txStatusVector.txTotalOnWire);
				DPrint(F(" ctf:")); DPrintBool(txStatusVector.txCtrlFrame);
				DPrint(F(" pcf:")); DPrintBool(txStatusVector.txPauseCtrlFrame);
				DPrint(F(" bkp:")); DPrintBool(txStatusVector.BackPressure);
				DPrint(F(" vla:")); DPrintBool(txStatusVector.txVLAN);
				DPrint(F(" pkm:")); DPrint(lastPktCapacity);
				DPrint(F(" mfb:")); DPrint(FreeMemoryMaxBlock());
				DPrint(F(" mfs:")); DPrint(FreeMemorySum());
				DNewline();
			}

			//----------------------------------------------------------

			Driver::Driver()
			{
			}

			Driver::Driver(const RamData& _macAddress)
			{
#if defined DEBUG && defined DEBUG_ASSERT
				if (ETH_RX_END % 2 == 0)
				{
					DPrint(F("* FixRdPtr() expects an odd ETH_RX_END")); DNewline();
				}
#endif								

				while (true)
				{
					InitSPI();

					SoftReset();

					// #2.2
					WaitAfterPoweron();

					auto revId = RevId();

#if defined DEBUG && defined DEBUG_ETH_DRIVER
					DPrint(F("ETH REVID="));
					DPrint(revId);
					DNewline();
					if (revId == 0) { DPrint(F("Reset failed")); DNewline(); }
#endif

					if (revId == B10 || // B1
						revId == B100 || // B4
						revId == B101 || // B5 
						revId == B110) // B7
						break;

					delay(1000);
				}

				SetupMemoryBuffer();

				SetMacAddress(_macAddress);
#if defined DEBUG && defined DEBUG_ETH_DRIVER
				DPrint("MAC: ");
				DPrintHex(ReadControlRegister(ETH_MAADR1)); DPrint('-');
				DPrintHex(ReadControlRegister(ETH_MAADR2)); DPrint('-');
				DPrintHex(ReadControlRegister(ETH_MAADR3)); DPrint('-');
				DPrintHex(ReadControlRegister(ETH_MAADR4)); DPrint('-');
				DPrintHex(ReadControlRegister(ETH_MAADR5)); DPrint('-');
				DPrintHex(ReadControlRegister(ETH_MAADR6)); DNewline();
#endif

				SetupRxFilter();

				DisableTxLoopback();

				EnableRx();

				while (LineStatus() == LineStatusEnum::LinkDown);
			}

			Driver::~Driver()
			{
			}

			const RamData& Driver::MacAddress() const { return macAddress; }

			// determine the line status
			LineStatusEnum Driver::LineStatus()
			{
				UpdateLineStatus();

				return lineStatus;
			}

			uint16_t Driver::Receive(byte *buf, uint16_t capacity)
			{
				lastPktCapacity = capacity;

				// #E6
				auto pktCnt = ReadControlRegister(ETH_EPKTCNT);

				if (pktCnt == 0) return 0;

				/*
				if (LineStatus() == LineStatusEnum::LinkDown)
				{
				#if defined DEBUG && defined DEBUG_ETH_RX
				DPrint(F("can't receive: link down")); DNewline();
				#endif
				return;
				}
				*/

#if defined DEBUG && defined DEBUG_ETH_RX
				DPrint(F("<-- RX")); DNewline();
#endif

#if defined DEBUG && defined DEBUG_ETH_RX && defined DEBUG_ETH_REGS
				DumpRegs();
#endif

#if defined DEBUG && defined DEBUG_ETH_RX
				DPrint(F("RXSTAT pktCnt:")); DPrint(pktCnt);
#if !defined DEBUG_ETH_RX_VERBOSE
				DNewline();
#endif
#endif

				if (nextPktPtr > ETH_RX_END)
				{
#if defined DEBUG && defined DEBUG_ETH_RX
					DPrint(F("* Invalid nextPtr=")); DPrintHex(nextPktPtr); DNewline();
#endif
					ResetRx();
					return 0;
				}

				// #4.2.2
				SetReadBufferMemoryPtr(nextPktPtr);

				// #7.2.2

				// Next Packet Pointer
				nextPktPtr = (uint16_t)ReadBufferMemory() | ((uint16_t)ReadBufferMemory() << 8);

				// Receive Status Vector
				ReadBufferMemory((byte *)&rxStatusVector, sizeof(rxStatusVector));

#if defined DEBUG && defined DEBUG_ETH_RX_VERBOSE
				DPrint(F(" nextPtr:")); DPrintHex(nextPktPtr); DPrint(' ');
				PrintRxStatusVector();
#endif

				auto len = rxStatusVector.receivedByteCount;

				if (rxStatusVector.receivedOk && !rxStatusVector.crcError && !rxStatusVector.lengthCheckError)
				{
					if (len > 0 && len <= capacity)
					{
						// Read data							
						ReadBufferMemory(buf, len);

#if defined DEBUG && defined DEBUG_ETH_RX_VERBOSE
						DPrintHex(buf, len, true); DNewline();
#endif

#if defined DEBUG && defined DEBUG_ETH2
						Eth2Print(Eth2GetHeader(buf));
#endif
					}
					else
					{
#if defined DEBUG && defined DEBUG_ETH_RX
						DPrint(F("* rx size invalid "));

						DPrint(len); DPrint('>'); DPrint(capacity); DNewline();
#endif
						len = 0;
					}
				}
				else
				{
#if defined DEBUG && defined DEBUG_ETH_RX
					DPrint(F("* rx flags invalid")); DNewline();
#endif
					len = 0;
				}

				auto _nextPktPtr = FixRdPtr(nextPktPtr);

				// #7.2.4 + #E14									
				WriteControlRegister(ETH_ERXRDPTL, lowByte(_nextPktPtr));
				WriteControlRegister(ETH_ERXRDPTH, highByte(_nextPktPtr));

				BitFieldSet(ETH_ECON2, ETH_ECON2_PKTDEC);

				return len;
			}			

			// transmit the packet ( before to fill the packet with the tx data call RxHandled if an rx packet was managed or FlushRx otherwise )			
			bool Driver::Transmit(const byte *buf, uint16_t len)
			{
				lastPktCapacity = len;

				/*
				if (LineStatus() == LineStatusEnum::LinkDown)
				{
#if defined DEBUG && defined DEBUG_ETH_TX
					DPrint(F("can't transmit: link down")); DNewline();
#endif
					return;
				}*/


#if defined DEBUG && defined DEBUG_ETH_TX
				DPrint(F("--> TX")); DNewline();
#endif								

#if defined DEBUG && defined DEBUG_ETH_TX
				if (len == 0)
				{
					DPrint(F("* tx len zero")); DNewline();
					return false;
				}
				if (len > (ETH_TX_END - ETH_TX_BEGIN))
				{
					DPrint(F("* tx len excessive")); DNewline();
					return false;
				}
#endif

				if (ReadControlRegister(ETH_EIR) & ETH_EIR_TXERIF)
				{
#if defined DEBUG && defined DEBUG_ETH_TX
					DPrint(F("* tx ERR")); DNewline();
#endif
					// #E12
					BitFieldSet(ETH_ECON1, ETH_ECON1_TXRTS);
					BitFieldClear(ETH_ECON1, ETH_ECON1_TXRTS);
					BitFieldClear(ETH_EIR, ETH_EIR_TXERIF);

					delay(20);
				}

				uint16_t txFrom = ETH_TX_BEGIN;
				uint16_t txTo = ETH_TX_BEGIN + len + 1;

				// #7.1.1
				WriteControlRegister(ETH_ETXSTL, lowByte(txFrom));
				WriteControlRegister(ETH_ETXSTH, highByte(txFrom));

				// #7.1.2
				SetWriteBufferMemoryPtr(txFrom);
				// control byte ( POVERRIDE=0 -> use of MACON3 )				
				WriteBufferMemory(0);
				WriteBufferMemory(buf, len);

				// #7.1.3
				WriteControlRegister(ETH_ETXNDL, lowByte(txTo));
				WriteControlRegister(ETH_ETXNDH, highByte(txTo));

#if defined DEBUG && defined DEBUG_ETH_TX_VERBOSE									 
				DPrint(F("tx req len=")); DPrint(len);
				DPrint(F(" [")); DPrintHex(txFrom);
				DPrint('-'); DPrintHex(txTo);
				DPrint(']'); DNewline();
#endif														

				// #7.1.4
				BitFieldClear(ETH_EIR, ETH_EIR_TXIF);

				// #7.1.5
				BitFieldSet(ETH_ECON1, ETH_ECON1_TXRTS);

				// wait transmission finish
				while (ReadControlRegister(ETH_ECON1) & ETH_ECON1_TXRTS);

#if defined DEBUG && defined DEBUG_ETH_TX_VERBOSE
				DPrintHex(buf, len, true); DNewline();
#endif

				bool err = false;

				if (ReadControlRegister(ETH_EIR) & ETH_EIR_TXIF)
				{
					auto estat = ReadControlRegister(ETH_ESTAT);
					if (estat & ETH_ESTAT_TXABRT)
					{
#if defined DEBUG && defined DEBUG_ETH_TX
						DPrint(F("* TxAbort")); DNewline();
#endif			
						err = true;
					}
					if (estat & ETH_ESTAT_LATECOL)
					{
#if defined DEBUG && defined DEBUG_ETH_TX
						DPrint(F("* LateCol")); DNewline();
#endif				
						err = true;
					}
				}
				else
				{
					// #12.1.4
					BitFieldClear(ETH_EIR, ETH_EIR_TXIF);
				}

				if (!err)
				{

#if defined DEBUG && defined DEBUG_ETH_TX_VERBOSE
					DPrint(F("waiting tx end")); DNewline();
#endif
					do
					{
						// read tx status vector
						SetReadBufferMemoryPtr(txTo + 1);
						ReadBufferMemory((byte *)&txStatusVector, sizeof(txStatusVector));
					} while (!txStatusVector.txDone);

#if defined DEBUG && defined DEBUG_ETH_TX_VERBOSE
					if (txStatusVector.txDone)
					{
						DPrint(F("TX done len=")); DNewline();
					}
					DPrint("TXSTAT "); PrintTxStatusVector();
#endif				
				}

#if defined DEBUG && defined DEBUG_ETH_TX && defined DEBUG_ETH_REGS
				DumpRegs();
#endif				

				if (err) 
					return false;
				else
					return true;
			}
			
		}

	}

}
