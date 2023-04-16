/*
This library is free software; you can redistribute it and /or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

See file LICENSE.txt for further informations on licensing terms.

*/

#include <ConfigurableFirmata.h>
#include "FirmataFeature.h"

bool FirmataFeature::handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value)
{
	// This handles the basic variables that are system and component independent
	if (variable_id == 0)
	{
		*value = 1;
		*data_type = SystemVariableDataType::Int;
		*status = SystemVariableError::NoError;
		return true;
	}
	if (variable_id == 1)
	{
		*value = MAX_DATA_BYTES;
		*data_type = SystemVariableDataType::Int;
		*status = SystemVariableError::NoError;
		return true;
	}

	return false;
}

