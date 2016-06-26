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
#include "ac_tlm_float.h"
#include <byteswap.h>

//////////////////////////////////////////////////////////////////////////////

float val1;
float val2;

/// Namespace to isolate float from ArchC
using user::ac_tlm_float;

/// Constructor
ac_tlm_float::ac_tlm_float( sc_module_name module_name , int k ) :
	sc_module( module_name ),
	target_export("iport")
{
	/// Binds target_export to the float
	target_export( *this );
}

/// Destructor
ac_tlm_float::~ac_tlm_float() {
}

/** Internal Write
 * Note: Always write 32 bits
 * @param a is the address to write
 * @param d id the data being write
 * @returns A TLM response packet with SUCCESS
 */
ac_tlm_rsp_status ac_tlm_float::writem( const uint32_t &a , const uint32_t &d )
{
	//cout << "writing... addr: " <<  std::hex  << a << " data: " << d << endl;
	if (a % 2 == 0) {
		val1 = d;
	}
	else {
		val2 == d;
	}
	return SUCCESS;
}

/** Internal Read
 * Note: Always read 32 bits
 * @param a is the address to read
 * @param d id the data that will be read
 * @returns A TLM response packet with SUCCESS and a modified d
 */
ac_tlm_rsp_status ac_tlm_float::readm( const uint32_t &a , uint32_t &d )
{
	// Calcula operação de soma
	if (a == 0x6600000) {
		d = val1 + val2;
	}
	// Calcula operação de subtração
	else if (a == 0x6700000) {
		d = val1 - val2;
	}
	// Calcula operação de multiplicação
	else if (a == 0x6800000) {
		d = val1 * val2;
	}
	// Calcula operação de divisão
	else if (a == 0x6900000) {
		d = val1 / val2;
	}
	//cout << "reading... addr: " << std::hex << a << " data: " << d << endl;
	return SUCCESS;
}


