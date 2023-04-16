/*
  FirmataExt.h - Firmata library
  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2009-2015 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#ifndef FirmataFeature_h
#define FirmataFeature_h

#include <ConfigurableFirmata.h>

class FirmataFeature
{
  public:
    virtual void handleCapability(byte pin) = 0;
    virtual boolean handlePinMode(byte pin, int mode) = 0;
    virtual boolean handleSysex(byte command, byte argc, byte* argv) = 0;
    virtual void reset() = 0;

    /// <summary>
    /// Regularly called by main thread
    /// </summary>
    /// <param name="elapsed">True if the default reporting time has elapsed, false otherwise. Components wishing to report status in
    /// a regular interval should not do anything if this is false.</param>
    virtual void report(bool elapsed)
    {
      // Empty by default
    }
    virtual ~FirmataFeature() = default;

    bool handleSystemVariableQuery(bool write, SystemVariableDataType* data_type, int variable_id, byte pin, SystemVariableError* status, int* value);
};

#endif
