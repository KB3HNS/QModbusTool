/**
 * \file metadata_structs.cpp
 * \brief Containers and definitions used by the program for metadata
 * \copyright
 * 2021 Andrew Buettner (ABi)
 *
 * \section LICENSE
 *
 * QModbusTool - A QT Based Modbus Client
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

//  c++ includes
/* -none- */

// C includes
/* -none- */

// project includes
#include "metadata_structs.h"  //  local include
#include "metadata_wrapper.h"  //  MetadataWrapper
#include <metadata.h>  //  modbus_plugin: DATA_BUFFER_REQUIRED_SIZE


Metadata::Metadata(const quint16 reg_num, void *const instance, const quint8 fc) :
    register_number{reg_num},
    label{},
    encoding{RegisterEncoding::ENCODING_UNKNOWN},
    min(false),
    max(false),
    dflt(false),
    function_code{qint8(fc)},
    m_request_instance{instance},
    m_request(DATA_BUFFER_REQUIRED_SIZE)
{
}


Metadata::~Metadata()
{
    auto inst = MetadataWrapper::get_instance();
    inst->dispose_metadata(this);
}
