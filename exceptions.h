/**
 * \file exceptions.h
 * \brief Exceptions used throughout the application
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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

//  c++ includes
#include <QString>  //  QString
#include <QException>  //  QException

// C includes
/* -none- */

// project includes
/* -none- */


/**
 * \brief Base class for all exceptions thrown by the application logic
 */
class AppException : public QException
{
public:

    /**
     * \brief constructor
     * @param reason Reason that the exception was thrown.
     */
    AppException(const QString &reason);

    /**
     * \brief Convert to a QString
     * @return String representation of this exception
     */
    operator QString () const;

    AppException* clone() const override;

    void raise() const override;

    const char* what() const noexcept override;

private:
    const QString text;
};


/**
 * \brief Exception thrown during file loading
 */
class FileLoadException : public AppException
{
public:

    /**
     * \brief constructor
     * @param reason Reason that the exception was thrown.
     * @param filename name of file being parsed
     */
    FileLoadException(const QString &reason, const QString &filename);

    /**
     * \brief constructor including line number
     * @param reason Reason that the exception was thrown.
     * @param filename name of file being parsed
     * @param line line number error encountered
     */
    FileLoadException(const QString &reason, const QString &filename, const size_t line);
};


#endif // EXCEPTIONS_H
