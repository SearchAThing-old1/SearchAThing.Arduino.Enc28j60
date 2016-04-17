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

#ifndef _SEARCHATHING_ARDUINO_ENC28J60_MISC_RXSTATUSVECTOR_H
#define _SEARCHATHING_ARDUINO_ENC28J60_MISC_RXSTATUSVECTOR_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <SearchAThing.Arduino.Utils\DebugMacros.h>
#include <SearchAThing.Arduino.Utils\RamData.h>

#include "RxStatusVector.h"

namespace SearchAThing
{

	namespace Arduino
	{

		namespace Enc28j60
		{

			// #7-3	
			typedef struct RxStatusVector
			{
				uint16_t receivedByteCount;				// 15-0
				uint16_t longEvtDropEvt : 1;			// 16
				uint16_t reserved0 : 1;					// 17
				uint16_t carrierEventSeen : 1;			// 18
				uint16_t reserved1 : 1;					// 19
				uint16_t crcError : 1;					// 20
				uint16_t lengthCheckError : 1;			// 21
				uint16_t lengthOutOfRange : 1;			// 22
				uint16_t receivedOk : 1;				// 23											
				uint16_t receivedMulticast : 1;			// 24
				uint16_t receivedBroadcast : 1;			// 25
				uint16_t dribbleNibble : 1;				// 26
				uint16_t receiveControlFrame : 1;		// 27
				uint16_t receivePauseControlFrame : 1;	// 28
				uint16_t receiveUnknownOpcode : 1;		// 29
				uint16_t receiveVLAN : 1;				// 30
				uint16_t zero : 1;						// 31																		
			};

		}

	}

}

#endif
