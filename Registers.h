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

#ifndef _SEARCHATHING_ARDUINO_ENC28J60_MISC_REGISTERS_H
#define _SEARCHATHING_ARDUINO_ENC28J60_MISC_REGISTERS_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <SearchAThing.Arduino.Utils\DebugMacros.h>

//----------------------------------------------------------------------

// Compact Register Address
// ~~~~~~~~~~~~~~~~~~~~~~~~
// encodes in a byte the address and the bank number
//
//   xBBBAAAAA
//
// - BBB is the bank nr (0,1,2,3)
// - AAAAA is the register address (0 to 0x1f)

#define ETH_CRA_BANK(x)	((byte)((x>>5) & B111))
#define ETH_CRA_REG(x)	((byte)(x & B11111))


namespace SearchAThing
{

	namespace Arduino
	{

		namespace Enc28j60
		{

			// NOTE : Not all register available are reported here
			//        mostly, only those really used by the current
			//        SearchAThing::Arduino::Enc28j60 implementation

			//----------------------------------------------------------
			// #4.1: SPI INSTRUCTION SET
			//----------------------------------------------------------

			// Read Control Register
			const byte ETH_SPIOP_RCR = B00000000;
			// Read Buffer Memory
			const byte ETH_SPIOP_RBM = B00111010;
			// Write Control Register
			const byte ETH_SPIOP_WCR = B01000000;
			// Write Buffer Memory
			const byte ETH_SPIOP_WBM = B01111010;
			// Bit Field Set
			const byte ETH_SPIOP_BFS = B10000000;
			// Bit Field Clear
			const byte ETH_SPIOP_BFC = B10100000;
			// System Reset Command (SoftReset)
			const byte ETH_SPIOP_SRC = B11111111;

			//----------------------------------------------------------
			// #3.1.1: ECON1 BSEL0, BSEL1
			//----------------------------------------------------------
			// CRA Compact Register Address : B0XYaaaaa
			// codification of BSEL0, BSEL1 into bits 5(X), 6(Y)
			// in order to describe with 1 byte either the bank
			// and the register number that is a 5bit = aaaaa
			//----------------------------------------------------------

			// BANK0 Registers CRA mask
			const byte ETH_BANK0 = B00000000;
			// BANK1 Registers CRA mask
			const byte ETH_BANK1 = B00100000;
			// BANK2 Registers CRA mask
			const byte ETH_BANK2 = B01000000;
			// BANK3 Registers CRA mask
			const byte ETH_BANK3 = B01100000;

			// special bit used to identify MAC, MII registers ( MA*, MI* ) #3.1
			const byte ETH_MAC_MII_FLAG = B10000000;

			//----------------------------------------------------------
			// Common banks registers
			//----------------------------------------------------------

			// reg. #12-2: Ethernet Interrupt Enable [REGISTER]
			const byte ETH_EIE = 0x1B;
			// Global INT Interrupt Enable
			const byte ETH_EIE_INTIE = (1 << 7);
			// Receive Packet Pending Interrupt Enable
			const byte ETH_EIE_PKTIE = (1 << 6);
			// DMA Interrupt Enable
			const byte ETH_EIE_DMAIE = (1 << 5);
			// Link Status Changed Interrupt Enable
			const byte ETH_EIE_LINKIE = (1 << 4);
			// Transmit Enable
			const byte ETH_EIE_TXIE = (1 << 3);
			// Transmit Error Interrupt Enable
			const byte ETH_EIE_TXERIE = (1 << 1);
			// Receive Error Interrupt Enable
			const byte ETH_EIE_RXERIE = (1 << 0);

			// reg. #12-3: Ethernet Interrupt Request (flag) [REGISTER]
			const byte ETH_EIR = 0x1C;
			// Receive Packet Panding Interrupt
			const byte ETH_EIR_PKTIF = (1 << 6);
			// DMA Interrupt
			const byte ETH_EIR_DMAIF = (1 << 5);
			// Link Change Interrupt
			const byte ETH_EIR_LINKIF = (1 << 4);
			// Transmit Interrupt
			const byte ETH_EIR_TXIF = (1 << 3);
			// Transmit Error Interrupt
			const byte ETH_EIR_TXERIF = (1 << 1);
			// Receive Error Interrupt
			const byte ETH_EIR_RXERIF = (1 << 0);

			// reg. #12-1: Ethernet Status [REGISTER]
			const byte ETH_ESTAT = 0x1D;
			// Interrupt
			const byte ETH_ESTAT_INT = (1 << 7);
			// Ethernet Buffer Error Status
			const byte ETH_ESTAT_BUFER = (1 << 6);
			// Late Collision Error
			const byte ETH_ESTAT_LATECOL = (1 << 4);
			// Receive Busy
			const byte ETH_ESTAT_RXBUSY = (1 << 2);
			// Transmit Abort
			const byte ETH_ESTAT_TXABRT = (1 << 1);
			// Clock Ready
			const byte ETH_ESTAT_CLKRDY = (1 << 0);

			// reg. #3-1: Ethernet Control [REGISTER] 1
			const byte ETH_ECON1 = 0x1F;
			// Transmit Logic Reset
			const byte ETH_ECON1_TXRST = (1 << 7);
			// Receive Logic Reset
			const byte ETH_ECON1_RXRST = (1 << 6);
			// DMA Start and Busy Status
			const byte ETH_ECON1_DMAST = (1 << 5);
			// DMA Checksum Enable
			const byte ETH_ECON1_CSUMEN = (1 << 4);
			// Transmit Request to Send
			const byte ETH_ECON1_TXRTS = (1 << 3);
			// Receive Enable
			const byte ETH_ECON1_RXEN = (1 << 2);
			// Bank Select bit1
			const byte ETH_ECON1_BSEL1 = (1 << 1);
			// Bank Select bit0
			const byte ETH_ECON1_BSEL0 = (1 << 0);

			// tbl. #3-2: Ethernet Control [REGISTER] 2
			const byte ETH_ECON2 = 0x1E;
			// Automatic Buffer Pointer Increment
			const byte ETH_ECON2_AUTOINC = (1 << 7);
			// Packet Decrement
			const byte ETH_ECON2_PKTDEC = (1 << 6);
			// Power Save Enable
			const byte ETH_ECON2_PWRSV = (1 << 5);
			// Voltage Regulator Power Save Enable
			const byte ETH_ECON2_VRPS = (1 << 3);

			//----------------------------------------------------------
			// Bank0 registers
			//----------------------------------------------------------

			//
			// #E5
			// BUFFER LAYOUT used
			//..........................................................
			// ERXSTH:ERXSTL = 0x0000 = 0000	RX Buffer Start
			// ERXRDPTH:ERXRDPTL				RX Buffer Read Pointer
			// ERDPTH:ERDPTL					Buffer Read Pointer			
			// ERXNDH:ERXNDL = 0x1A11 = 6673	RX Buffer End
			//
			// ETXSTH:ETXSTL = 0x1A12 = 6674	TX Buffer Start
			// EWRPTH:EWRPTL					Buffer Writer Pointer
			// ETXNDH:ETXNDL = 0x1FFF = 8191	TX Buffer End
			//
			// Where:
			// - TX Buffer Size = MAX_FRAME_LENGTH	= 1518 bytes
			// - RX Buffer Size = 8192 - 1518		= 6674 bytes
			//
			// Default library Ethernet Packet RAM SIZE = PACKET_SIZE = 600			
			//

			//

			// #4.2.2: Buffer Read Pointer [REGISTER] (low byte)
			const byte ETH_ERDPTL = 0x00;

			// #4.2.2: Buffer Read Pointer [REGISTER] (high byte)
			const byte ETH_ERDPTH = 0x01;

			// #4.2.4: Buffer Write Pointer [REGISTER] (low byte)
			const byte ETH_EWRPTL = 0x02;

			// #4.2.4: Buffer Write Pointer [REGISTER] (high byte)
			const byte ETH_EWRPTH = 0x03;

			//

			// fig. #3-2: TX Buffer Start [REGISTER] (low byte)
			const byte ETH_ETXSTL = 0x04;

			// fig. #3-2: TX Buffer Start [REGISTER] (high byte)
			const byte ETH_ETXSTH = 0x05;

			// fig. #3-2: TX Buffer End [REGISTER] (low byte)
			const byte ETH_ETXNDL = 0x06;

			// fig. #3-2: TX Buffer End [REGISTER] (high byte)
			const byte ETH_ETXNDH = 0x07;

			//

			// fig. #3-2: RX Buffer Start [REGISTER] (low byte)
			const byte ETH_ERXSTL = 0x08;

			// fig. #3.2: RX Buffer Start [REGISTER] (high byte)
			const byte ETH_ERXSTH = 0x09;

			// fig. #3.2: RX Buffer End [REGISTER] (low byte)
			const byte ETH_ERXNDL = 0x0A;

			// fig. #3.2: RX Buffer End [REGISTER] (high byte)
			const byte ETH_ERXNDH = 0x0B;

			//

			// #7.2.4: RX Buffer Read Pointer [REGISTER] (low byte)
			const byte ETH_ERXRDPTL = 0x0C;

			// #7.2.4: RX Buffer Read Pointer [REGISTER] (high byte)
			const byte ETH_ERXRDPTH = 0x0D;

			//----------------------------------------------------------
			// Bank1 banks registers
			//----------------------------------------------------------

			// #8.2: Pattern Match Mask [REGISTER] byte 0
			const byte ETH_EPMM0 = (ETH_BANK1 | 0x08);

			// #8.2: Pattern Match Mask [REGISTER] byte 1
			const byte ETH_EPMM1 = (ETH_BANK1 | 0x09);

			// #8.2: Pattern Match Checksum [REGISTER] (low byte)
			const byte ETH_EPMCSL = (ETH_BANK1 | 0x10);

			// #8.2: Pattern Match Checksum [REGISTER] (high byte)
			const byte ETH_EPMCSH = (ETH_BANK1 | 0x11);

			// reg. #8-1: Ehternet Receive Filter Control [REGISTER]
			const byte ETH_ERXFCON = (ETH_BANK1 | 0x18);
			// Unicast Filter Enable
			const byte ETH_ERXFCON_UCEN = (1 << 7);
			// AND/OR Filter Select
			const byte ETH_ERXFCON_ANDOR = (1 << 6);
			// Post-Filter CRC Check Enable
			const byte ETH_ERXFCON_CRCEN = (1 << 5);
			// Pattern Match Filter Enable
			const byte ETH_ERXFCON_PMEN = (1 << 4);
			// Magic Packet(TM) Filter Enable
			const byte ETH_ERXFCON_MPEN = (1 << 3);
			// Hash Table Filter Enable
			const byte ETH_ERXFCON_HTEN = (1 << 2);
			// Multicast Filter Enable
			const byte ETH_ERXFCON_MCEN = (1 << 1);
			// Broadcast Filter Enable
			const byte ETH_ERXFCON_BCEN = (1 << 0);

			// 7.2: Ethernet Packet Count [REGISTER]
			const byte ETH_EPKTCNT = (ETH_BANK1 | 0x19);

			//----------------------------------------------------------
			// Bank2 banks registers
			//----------------------------------------------------------

			// reg. #6-1: MAC Control [REGISTER] 1
			const byte ETH_MACON1 = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x00);
			// Pause Control Frame Transmission Enable
			const byte ETH_MACON1_TXPAUS = (1 << 3);
			// Pause Control Frame Reception Enable
			const byte ETH_MACON1_RXPAUS = (1 << 2);
			// MAC Receive Enable
			const byte ETH_MACON1_MARXEN = (1 << 0);

			// reg. #6-3: MAC Control [REGISTER] 3
			const byte ETH_MACON3 = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x02);
			// Automatic Pad and CRC Configuration bits
			const byte ETH_MACON3_PADCFG0 = (1 << 5);
			// Transmit CRC Enable bit
			const byte ETH_MACON3_TXCRCEN = (1 << 4);
			// Frame Length Checking Enable
			const byte ETH_MACON3_FRMLNEN = (1 << 1);
			// MAC Full-Duplex Enable
			const byte ETH_MACON3_FULDPX = (1 << 0);

			// #6.5: Maximum Frame Length [REGISTER] (low byte)
			const byte ETH_MAMXFL = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x0A);
			// #6.5: Maxumyn Frame Length [REGISTER] (high byte)
			const byte ETH_MAMXFH = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x0B);

			// reg. #6-4: MAC Back-to-Back Inter-Packet GAP [REGISTER]
			const byte ETH_MABBIPG = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x04);

			// #6-5: non-back-to-back Inter-Packet GAP [REGISTER] (low byte)
			const byte ETH_MAIPGL = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x06);

			// #6-5: non-back-to-back Inter-Packet GAP [REGISTER] (high byte)
			const byte ETH_MAIPGH = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x07);

			// reg. #3-3: MII Command [REGISTER]
			const byte ETH_MICMD = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x12);
			// MII Read Enable
			const byte ETH_MICMD_MIIRD = (1 << 0);

			// #3.3.2: MII Write Data [REGISTER] (low byte)
			const byte ETH_MIWRL = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x16);

			// #3.3.2: MII Write Data [REGISTER] (high byte)
			const byte ETH_MIWRH = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x17);

			// #3.3.1: MII Read Data [REGISTER] (low byte)
			const byte ETH_MIRDL = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x18);

			// #3.3.1: MII Read Data [REGISTER] (high byte)
			const byte ETH_MIRDH = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x19);

			// #3.3.1: MII Register Address [REGISTER]
			const byte ETH_MIREGADR = (ETH_MAC_MII_FLAG | ETH_BANK2 | 0x14);

			//----------------------------------------------------------
			// Bank3 banks registers
			//----------------------------------------------------------

			// #6.5: MAC Address Initialization (byte 5) [REGISTER]
			const byte ETH_MAADR5 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x00);

			// #6.5: MAC Address Initialization (byte 6) [REGISTER]
			const byte ETH_MAADR6 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x01);

			// #6.5: MAC Address Initialization (byte 3) [REGISTER]
			const byte ETH_MAADR3 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x02);

			// #6.5: MAC Address Initialization (byte 4) [REGISTER]
			const byte ETH_MAADR4 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x03);

			// #6.5: MAC Address Initialization (byte 1) [REGISTER]
			const byte ETH_MAADR1 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x04);

			// #6.5: MAC Address Initialization (byte 2) [REGISTER]
			const byte ETH_MAADR2 = (ETH_MAC_MII_FLAG | ETH_BANK3 | 0x05);

			// reg. #3-4: MII Status [REGISTER]
			const byte ETH_MISTAT = (ETH_BANK3 | 0x0A);
			// MII Management Busy
			const byte ETH_MISTAT_BUSY = (1 << 0);

			// #3.3.5: Ethernet Revision ID [REGISTER]
			const byte ETH_EREVID = (ETH_BANK3 | 0x12);

			// reg. #11-1: PHY Control [REGISTER] 1
			const byte ETH_PHCON1 = (0x00);
			// PHY Loopback bit
			const uint16_t ETH_PHCON1_PLOOPBK = (1 << 14);

			// reg. #3-5: Physical Layer Status [REGISTER] 1
			const byte ETH_PHSTAT1 = (0x01);
			// PHY Latching Link Status
			const uint16_t ETH_PHSTAT1_LLSTAT = (1 << 2);

			// reg. 6-5: PHY Control [REGISTER] 2
			const byte ETH_PHCON2 = (0x10);
			// PHY Half-Duplex Loopback Disable
			const uint16_t ETH_PHCON2_HDLDIS = (1 << 8);

			// reg. #3-5: Physical Layer Status [REGISTER] 2
			const byte ETH_PHSTAT2 = (0x11);
			// PHY Link Status bit (non-latching)
			const uint16_t ETH_PHSTAT2_LSTAT = (1 << 10);

			// reg. #12-5: PHY Interrupt Request (flag) [REGISTER]
			const byte ETH_PHIR = (0x13);
			// Link Change Interrupt
			const uint16_t ETH_PHIR_PLNKIF = (1 << 4);

		}

	}

}

#endif
