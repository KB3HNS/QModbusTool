/**
 * \file main.cpp
 * \brief Application entry point
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
 *
 *
 * \section DESCRIPTION
 *
 * QModbusTool is a general-purpose modbus polling system based on libmodbus.
 * QModbusTool is intended to be a straight-forward implementation of a modbus
 * viewer supporting session and register set save and restore as well as
 * simple poll-driven trending.  QModbusTool has been release under the GPL-2
 * license and includes properietary decoding logic for polling metadata.  A
 * proprietary license may be available by contacting the author:
 * Andrew Buettner: leeloo <at> cletis <dot> net
 */

//  c++ includes
#include <QApplication>  //  QApplication

// C includes
/* -none- */

// project includes
#include "mainwindow.h"  //  MainWindow


/**
 * \brief Main entry point
 *
 * @param argc standard argument
 * @param argv standard argument
 *
 * @return exit code at exit
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
