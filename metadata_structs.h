/**
 * \file metadata_structs.h
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

#ifndef METADATA_STRUCTS_H
#define METADATA_STRUCTS_H

//  c++ includes
#include <QtCore>  //  quint8 and friends
#include <optional>  //  std::optional
#include <memory>  //  std::shared_ptr

// C includes
/* -none- */

// project includes
/* -none- */


// /////////////////////////////////////////////////////////////////////////////
// Forward declarations
// /////////////////////////////////////////////////////////////////////////////
class BaseDialog;


/**
 * \brief Encoding types recognized in the proprietary "read metadata" request
 * \note
 * These must match definition in project
 */
enum RegisterEncoding : qint8 {
    ENCODING_NONE=0,
    ENCODING_UINT16,
    ENCODINT_INT16,
    ENCODING_SIGNED_BYTES,
    ENCODING_BYTES,
    ENCODING_BITS,
    ENCODING_USER,
    ENCODING_UNKNOWN
};


/**
 * \brief All data that can be returned by querying the register metadata
 */
class Metadata {
    friend class MetadataWrapper;

public:
    /**
     * \brief default constructor
     * \note
     * This function shall only be called by the Metadata wrapper
     *
     * @param reg_num Register number
     * @param instance plugin instance value
     * @param fc function code
     */
    Metadata(const quint16 reg_num, void *const instance, const quint8 fc);
    Metadata(const Metadata&) = delete;

    const quint16 register_number; /**< Register numer */
    QString label; /**< Brief discription */
    RegisterEncoding encoding; /**< Data encoding method */
    std::optional<qint32> min; /**< Minimum allowed range */
    std::optional<qint32> max; /**< Maximum allowed range */
    std::optional<qint32> dflt; /**< register default value */
    const qint8 function_code; /**< function code of the request */

    ~Metadata();

private:
    void *const m_request_instance;
    std::vector<quint8> m_request;

};


/**
 * \brief State tracking of a sequence of metadata requests
 */
struct WindowMetadataRequest {
    quint16 current_register; /**< Currently polled register */
    quint16 last_register; /**< Last register to be polled in sequence */
    quint8 node; /**< Node to poll */
    BaseDialog *requester; /**< Pointer to window requesting */
    std::shared_ptr<Metadata> request = nullptr; /**< Pointer to container */
};


#endif // METADATA_STRUCTS_H
