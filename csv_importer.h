/**
 * \file csv_importer.h
 * \brief Import CSV data interactive modal dialog.
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
 * \section DESCRIPTION
 *
 * A complex dialog to allow the user to select columns containing data and
 * import them.
 */

#ifndef CSV_IMPORTER_H
#define CSV_IMPORTER_H

//  c++ includes
#include <QList>  //  QList
#include <QStringList>  //  QStringList
#include <QGridLayout>  //  QGridLayout
#include <QTableWidget>  //  QTableWidget
#include <QPushButton>  //  QPushButton
#include <QComboBox>  //  QComboBox
#include <QCheckBox>  //  QCheckBox
#include <QSpinBox>  //  QSpinBox
#include <vector>  //  std::vector
#include <array>  //  std::array
#include <tuple>  //  std::tuple
#include <utility>  //  std::pair

// C includes
/* -none- */

// project includes
#include "base_dialog.h"


/**
 * \brief Field data configured by this dialog box.
 */
using FieldData = std::tuple<size_t, size_t, ssize_t, bool>;


/**
 * \brief Field -> data mapping in ``FieldData`` tuple
 */
enum TestFields : size_t {
    REG_NUMBER=0,
    REG_VALUE,
    NODE_ID,
    HEADER_ROW
};


/**
 * \brief CSV Data Import Selection
 */
class CsvImporter : public BaseDialog
{
    Q_OBJECT

public:

    /**
     * \brief constructor
     * @param parent parent QObject owner
     * @param all_data Data decoded by the CSV parser
     */
    CsvImporter(QWidget *parent, const QList<QStringList> &all_data);

    /**
     * \brief Get the configuration specified.
     * @throws AppException if the dialog is not "finished"
     * @return tuple representing fields \sa TestFields
     */
    [[nodiscard]] const FieldData get_config() const;

private slots:

    /**
     * \brief OK Button clicked - verify input
     */
    void on_ok_clicked();

    /**
     * \brief Use fixed node checked / unchecked
     */
    void on_single_node_checked(int status);

    /**
     * \brief First row contains headers checked / unchecked
     */
    void on_header_row_checked(int status);

    /**
     * \brief Attempt to automatically deduce fields from input
     * \note
     * Made a slot so it can be directly invoked from a QTimer::singleShot
     */
    void attempt_autoconfig();

private:

    /**
     * \brief Initialize the UI components.
     */
    void setupUi() override;

    /**
     * \brief Called when a column header is checked/unchecked.
     * @param checkbox Widget that generated the event
     * @param column column that has been updated
     */
    void on_column_checked(const QCheckBox *const checkbox, const size_t column);


    bool m_is_valid = false;
    QTableWidget *const m_preview_table;
    QWidget *const m_grid_container;
    QGridLayout *const m_control_grid;
    QPushButton *const m_ok;
    QPushButton *const m_cancel;
    QCheckBox *const m_first_row_headers;
    QCheckBox *const m_fixed_node;
    QSpinBox *const m_node_select;

    /**
     * \var m_test_text
     * Contains the first few rows of data in column-major order.
     */
    std::vector<std::array<QString, 5>> m_test_text;
    std::vector<std::pair<QComboBox*, QCheckBox*>> m_role_selection;

    /* Order must match that in the tuple */
    const QStringList m_options;

    /* Only updated when ok clicked is successful */
    FieldData m_config;
};

#endif // CSV_IMPORTER_H
