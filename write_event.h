/**
 * \file write_event.h
 * \brief Write requests shared object.
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

#ifndef WRITE_EVENT_H
#define WRITE_EVENT_H

//  c++ includes
#include <vector>  //  std::vector
#include <QTypeInfo>  //  quint8, quint16

// C includes
/* -none- */

// project includes
/* -none- */


// /////////////////////////////////////////////////////////////////////////////
// Forward declarations
// /////////////////////////////////////////////////////////////////////////////
class BaseDialog;


/**
 * \brief Structure outlining a write request
 */
struct WriteRequest {
    BaseDialog *requester; /**< Request source */

    quint8 node; /**< Send request to node */

    quint16 first_register; /**< Starting register number (eg: 42, 40023) */

    std::vector<quint16> values; /**< List of values to be written */
};


#endif // WRITE_EVENT_H
