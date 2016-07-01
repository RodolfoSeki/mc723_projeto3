//////////////////////////////////////////////////////////////////////////////
// Standard includes
// SystemC includes
// ArchC includes

#include "ac_tlm_router.h"

//////////////////////////////////////////////////////////////////////////////

/// Namespace to isolate memory from ArchC
using user::ac_tlm_router;

/// Constructor
ac_tlm_router::ac_tlm_router( sc_module_name module_name ) :
  sc_module( module_name ),
  target_export("iport"),
  MEM_port("MEM_port", 104857600U), //100 M
  TRIG_ACC_port("TRIG_ACC_port",4U), // 4 B
  FLOAT_ACC_port("FLOAT_ACC_port",4U), // 4 B
  PERIPHERAL_port("PERIPHERAL_port",4U) // 4 B
{
    /// Binds target_export to the memory
    target_export( *this );
}

ac_tlm_rsp ac_tlm_router::transport( const ac_tlm_req &request ) {

	if((request.addr < 0x6400000))
	{
		return MEM_port->transport(request);
	}
	// Acesso ao lock 0x6400000 - 0x6500000
	
	else if (request.addr < 0x6500000) {
		return PERIPHERAL_port->transport(request);
	}
	// Acesso ao acelerador de ponto flutuante 0x6500000 - 0x6600000
	else if (request.addr < 0x6600000) {
		return TRIG_ACC_port->transport(request);
	}
	// Acesso ao acelerador de operações trigonométricas 0x6600000 - 0x6700000
	else {
		return FLOAT_ACC_port->transport(request);
	}

}

/// Destructor
ac_tlm_router::~ac_tlm_router() {
}
