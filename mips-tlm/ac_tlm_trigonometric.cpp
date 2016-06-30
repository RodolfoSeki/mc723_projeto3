/**
 * @file      ac_tlm_mem.cpp
 * @author    Bruno de Carvalho Albertini
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   0.1
 * @date      Sun, 02 Apr 2006 08:07:46 -0200
 *
 * @brief     Implements a ac_tlm memory.
 *
 * @attention Copyright (C) 2002-2005 --- The ArchC Team
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *
 */

//////////////////////////////////////////////////////////////////////////////
// Standard includes
// SystemC includes
// ArchC includes

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ac_tlm_trigonometric.h"
#include <netinet/in.h>
#include <byteswap.h>

//////////////////////////////////////////////////////////////////////////////

float value;

/// Namespace to isolate trigonometric from ArchC
using user::ac_tlm_trigonometric;

/// Constructor
ac_tlm_trigonometric::ac_tlm_trigonometric( sc_module_name module_name , int k ) :
	sc_module( module_name ),
	target_export("iport")
{
	/// Binds target_export to the trigonometric
	target_export( *this );
}

/// Destructor
ac_tlm_trigonometric::~ac_tlm_trigonometric() {
}

/** Internal Write
 * Note: Always write 32 bits
 * @param a is the address to write
 * @param d id the data being write
 * @returns A TLM response packet with SUCCESS
 */
ac_tlm_rsp_status ac_tlm_trigonometric::writem( const uint32_t &a , const uint32_t &d )
{
	//cout << "writing... addr: " <<  std::hex  << a << " data: " << d << endl;
	uint32_t aux = ntohl(d);
	value = *((float*) &aux);
	return SUCCESS;
}

/** Internal Read
 * Note: Always read 32 bits
 * @param a is the address to read
 * @param d id the data that will be read
 * @returns A TLM response packet with SUCCESS and a modified d
 */
ac_tlm_rsp_status ac_tlm_trigonometric::readm( const uint32_t &a , uint32_t &d )
{
	//cout << "reading... addr: " << std::hex << a << " data: " << d << endl;
	// Calcula coseno
	if (a == 0x6500000) {
		value = cos(value);
		uint32_t aux = *((uint32_t *) &value);
                d = htonl(aux);
	}
	// Calcula seno
	else {
		value = sin(value);
		uint32_t aux = *((uint32_t *) &value);
                d = htonl(aux);
	}
	return SUCCESS;
}


