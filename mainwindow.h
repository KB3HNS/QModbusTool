/**
 * \file mainwindow.h
 * \brief Main window for program
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//  c++ includes
#include <unordered_set>  //  std::unordered_set
#include <QList>  //  QList
#include <QMainWindow>  //  QMainWindow
#include <QTimer>  //  QTimer
#include <QDomDocument>  //  QDomDocument
#include <QDomElement>  //  QDomElement, QDomNode
#include <QStringList>  //  QStringList

// C includes
/* -none- */

// project includes
#include "register_display.h"  //  RegisterDisplay
#include "modbusthread.h"  //  ModbusThread
#include "write_event.h"  //  WriteRequest
#include "scheduler.h"  //  Scheduler
#include "ui_mainwindow.h"  //  Ui::MainWindow
#include "trend_window.h"  //  TrendWindow
#include "base_dialog.h"  //  BaseDialog


/**
 * \brief The main window for application
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     */
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    virtual void closeEvent(QCloseEvent *evt) override;

private slots:

    /**
     * \brief Signal Connect menu item triggered.
     */
    void on_actionConnect_triggered();

    /**
     * \brief Signal New Coils menu item triggered.
     */
    void on_actionCoils_triggered();

    /**
     * \brief Signal New Inputs menu item triggered.
     */
    void on_actionInputs_triggered();

    /**
     * \brief Signal New Input Registers menu item triggered.
     */
    void on_actionInput_Registers_triggered();

    /**
     * \brief Signal New Holding Registers menu item triggered.
     */
    void on_actionHolding_Registers_triggered();

    /**
     * \brief Signal modbus exception occurred.
     * @param requester Originating reqest window
     * @param exception exception text
     */
    void modbus_on_error(BaseDialog *const requester, const QString exception);

    /**
     * \brief Signal modbus poll complete (from modbus thread).
     */
    void modbus_on_data();

    /**
     * @brief Signal modbus error (from modbus thread).
     * @param error_code modbus error code
     */
    void modbus_on_error_protocol(const int error_code);

    /**
     * \brief Signal Poll Once menu item triggered.
     */
    void on_actionOnce_triggered();

    /**
     * \brief Signal Poll Continuous menu item toggled.
     */
    void on_actionContinuous_triggered();

    /**
     * \brief Signal that a child window was closed.
     * @param window reference window
     */
    void register_window_destroyed(BaseDialog *window);

    /**
     * \brief Signal to periodically refresh the status bar (timer).
     */
    void update_timer_on_expired();

    /**
     * \brief Signal that standard polling is complete (scheduler).
     */
    void polling_on_complete();

    /**
     * \brief Signal save current session menu item triggered.
     */
    void on_actionSave_Session_triggered();

    /**
     * \brief Signal restore previously saved session menu item triggered.
     */
    void on_actionRestore_Session_triggered();

    /**
     * \brief Signal read metadata menu item triggered.
     */
    void on_actionRead_Metadata_triggered();

    /**
     * \brief Signal read metadata menu item triggered.
     */
    void on_actionLoad_Register_Data_triggered();

    /**
     * \brief Signal new Trend menu item triggered.
     */
    void on_actionTrend_triggered();

    /**
     * \brief Signal that the trend window has been closed.
     */
    void trend_on_closed(BaseDialog *w);

private:

    /**
     * \brief After connection completed logic.
     */
    void post_connected();

    /**
     * \brief After Modbus thread closed logic.
     */
    void post_disconnected();

    /**
     * \brief Save configuration
     *
     * @return XML tree to write to file.
     */
    [[nodiscard]] QDomDocument save_config() const;

    /**
     * \brief Load configuration
     * @throws FileLoadException on error
     * @param filename file to load
     */
    void load_config(const QString &filename);

    /**
     * \brief Configure a new register display window.
     *
     * @param window window to be added
     */
    void add_window(RegisterDisplay *const window);

    /**
     * \brief Load the "communications" section of the configuration.
     * @throws AppException on error
     * @param node Communications root node reference
     */
    void load_communications_config(const QDomElement &node);

    /**
     * \brief Load the individual windows section of the configuration.
     * @throws AppException on error
     * @param node Windows root node reference
     */
    void load_windows(const QDomNode &node);

    /**
     * \brief Load the individual windows section of the configuration.
     * @throws AppException on error
     * @param node Window XML element node.
     * @return tuple {slave id, start register, register count}
     */
    [[nodiscard]] std::tuple<quint8, quint16, quint16>
        load_base_data(const QDomElement &node) const;

    /**
     * \brief Load data from CSV and disseminate to components (windows, protocol layer)
     * @param all_data parsed array of CSV data
     * @param value_index index of field to interpret as the "value"
     * @param register_index index of field to interpret as the "register number"
     * @param node_index when >= 0 index of the field to interpret as the "Device ID"
     *                   when < 0 the absolute value is \f node + 1 \f
     * @param skip_first_row set ``true`` to skip the first row
     */
    void load_csv_data(const QList<QStringList> &all_data,
                       const size_t value_index,
                       const size_t register_index,
                       const ssize_t node_index,
                       const bool skip_first_row);

    /**
     * \brief Append data to an outgoind write request
     * @param wr reference to the write request
     * @param reg register number
     * @param value value to write
     * @param node Device ID receiving the write
     */
    void append_write(WriteRequest &wr, const quint16 reg, const quint16 value, const quint8 node);

    Ui::MainWindow *const m_ui;
    bool m_connected;
    bool m_connecting=false;
    bool m_connecting2=false;
    bool m_active=false;
    std::unordered_set<RegisterDisplay*> m_register_windows;
    ModbusThread *m_engine=nullptr;
    Scheduler *const m_scheduler;
    QTimer *const m_update_timer;
    TrendWindow *m_trend;
};


#endif // MAINWINDOW_H
