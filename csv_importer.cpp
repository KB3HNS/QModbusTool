/**
 * \file csv_importer.cpp
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
 */


//  c++ includes
#include <QMessageBox>  //  QMessageBox
#include <QRegExp>  //  QRegExp
#include <QTimer> //  QTimer
#include <optional>  //  std::optional
#include <algorithm>  //  std::fill
#include <unordered_set>  //  std::unordered_set

// C includes
/* -none- */

// project includes
#include "csv_importer.h"  //  local include
#include "exceptions.h"  //  AppException


namespace {
    const auto g_enabled_tint = 0xFF;
    const auto g_disabled_tint = 0xE0;
    const auto g_disabled_color = QColor(g_disabled_tint, g_disabled_tint, g_disabled_tint);
    const auto g_enabled_color = QColor(g_enabled_tint, g_enabled_tint, g_enabled_tint);
    const auto g_header_color = QColor(g_disabled_tint, g_disabled_tint, g_enabled_tint);

}  //  Anonymous namespace


CsvImporter::CsvImporter(QWidget *parent, const QList<QStringList> &all_data)
    : BaseDialog(parent),
      m_preview_table{new QTableWidget(7, all_data[0].size(), this)},
      m_grid_container{new QWidget(this)},
      m_control_grid{new QGridLayout(m_grid_container)},
      m_ok{new QPushButton(tr("Ok"), m_grid_container)},
      m_cancel{new QPushButton(tr("Cancel"), m_grid_container)},
      m_first_row_headers{new QCheckBox(tr("First row contains headers?"), m_grid_container)},
      m_fixed_node{new QCheckBox(tr("Use fixed node?"), m_grid_container)},
      m_node_select{new QSpinBox(m_grid_container)},
      m_test_text(size_t(all_data[0].size())),
      m_role_selection(size_t(all_data[0].size())),
      m_options{tr("Register Number"), tr("Value"), tr("Device ID/Node")},
      m_config{}
{
    for (auto i=all_data.begin(); all_data.end() != i; ++i) {
        const auto row = size_t(std::distance(all_data.begin(), i));
        if (row >= m_test_text[0].size()) {
            break;
        }

        for (auto j=i->begin(); i->end() != j; ++j) {
            const auto col = size_t(std::distance(i->begin(), j));
            if (col < m_test_text.size()) {
                m_test_text[col][row] = *j;
            }
        }
    }

    connect(m_ok, &QPushButton::clicked, this, &CsvImporter::on_ok_clicked);
    connect(m_cancel, &QPushButton::clicked, this, &CsvImporter::close);

    m_node_select->setRange(0, 255);
    m_node_select->setEnabled(false);
    connect(m_first_row_headers, &QCheckBox::stateChanged, this, &CsvImporter::on_header_row_checked);
    connect(m_fixed_node, &QCheckBox::stateChanged, this, &CsvImporter::on_single_node_checked);
}


void CsvImporter::setupUi()
{
    m_top_layout->addWidget(m_preview_table);
    m_top_layout->addWidget(m_grid_container);
    m_top_layout->setSizeConstraint(QLayout::SetMinimumSize);
    m_top_layout->setContentsMargins(0, 0, 0, 0);
    setContentsMargins(0, 0, 0, 0);

    m_preview_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_preview_table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    for (size_t col=0; col<m_test_text.size(); ++col) {
        auto check = new QCheckBox(tr("Use Column"));
        m_preview_table->setCellWidget(0, int(col), check);
        connect(check, &QCheckBox::stateChanged, this, [=](int status) {
            static_cast<void>(status);
            on_column_checked(check, col);
        });

        auto selection = new QComboBox(m_preview_table);
        selection->addItems(m_options);
        selection->setEnabled(false);
        m_preview_table->setCellWidget(1, int(col), selection);
        m_role_selection[col] = {selection, check};

        for (size_t row=0; row<m_test_text[col].size(); ++row) {
            auto item = new QTableWidgetItem(m_test_text[col][row]);
            item->setFlags(Qt::NoItemFlags);
            item->setBackgroundColor(g_disabled_color);
            item->setTextColor(QColor(0, 0, 0));
            m_preview_table->setItem(int(row + 2U), int(col), item);
        }
    }

    m_grid_container->setLayout(m_control_grid);
    m_control_grid->addWidget(m_first_row_headers, 0, 0);
    m_control_grid->addWidget(m_fixed_node, 0, 1);
    m_control_grid->addWidget(m_node_select, 0, 2);
    m_control_grid->addWidget(m_ok, 1, 0);
    add_icon_to_button(m_ok, QStyle::SP_DialogApplyButton);
    m_control_grid->addWidget(m_cancel, 1, 2);
    add_icon_to_button(m_cancel, QStyle::SP_DialogCloseButton);
    m_preview_table->show();
    m_grid_container->show();
    m_preview_table->resizeColumnsToContents();
    resize(477, 323);
    setWindowTitle(tr("Import CSV Data"));
    QTimer::singleShot(160, this, &CsvImporter::attempt_autoconfig);
}


void CsvImporter::on_column_checked(const QCheckBox *const checkbox, const size_t column)
{
    m_role_selection[column].first->setEnabled(checkbox->isChecked());
    for (size_t i=0U; i<m_test_text[column].size(); ++i) {
        auto color = g_disabled_color;
        if ((0U == i) && m_first_row_headers->isChecked()) {
            color = g_header_color;
        } else if (checkbox->isChecked())  {
            color = g_enabled_color;
        } else {

        }
        auto w = m_preview_table->item(int(i+2U), int(column));
        w->setBackgroundColor(color);
    }
}


void CsvImporter::on_ok_clicked()
{
    std::optional<ssize_t> node{};
    std::optional<size_t> register_index{};
    std::optional<size_t> value_index{};
    if (m_fixed_node->isChecked()) {
        auto node_val = m_node_select->value() * -1;
        node = node_val - 1;
    }

    QStringList error_text;
    for (size_t i=0U; i<m_test_text.size(); ++i) {
        if (m_role_selection[i].second->isChecked()) {
            switch (m_role_selection[i].first->currentIndex()) {
            case 0:
                if (register_index.has_value()) {
                    error_text << tr("Duplicate register column %i").arg(i);
                } else {
                    register_index = i;
                }
                break;

            case 1:
                if (value_index.has_value()) {
                    error_text << tr("Duplicate value column %i").arg(i);
                } else {
                    value_index = i;
                }
                break;

            case 2:
                if (node.has_value()) {
                    error_text << tr("Duplicate node column %i").arg(i);
                } else {
                    node = ssize_t(i);
                }
                break;

            default:
                error_text << tr("Illegal selection on row %i").arg(i);
            }
        }
    }

    if (!register_index.has_value()) {
        error_text << tr("No register column");
    }

    if (!value_index.has_value()) {
        error_text << tr("No value column");
    }

    if (!node.has_value()) {
        error_text << tr("No node");
    }

    if (error_text.size() == 0) {
        m_config = {register_index.value(),
                    value_index.value(),
                    node.value(),
                    m_first_row_headers->isChecked()};
        m_is_valid = true;
        emit accept();
    } else {
        auto error_box = QMessageBox(this);
        error_box.setIcon(QMessageBox::Critical);
        error_box.setStandardButtons(QMessageBox::Ok);
        error_box.setText(tr("Invalid import configuration specified"));
        error_box.setInformativeText(tr("Import errors were reported"));
        error_box.setWindowTitle(tr("Import error"));
        error_box.setDetailedText(error_text.join('\n'));
        error_box.exec();
    }
}


void CsvImporter::on_single_node_checked(int status)
{
    static_cast<void>(status);
    m_node_select->setEnabled(m_fixed_node->isChecked());
}


void CsvImporter::on_header_row_checked(int status)
{
    static_cast<void>(status);
    for (size_t i=0U; i<m_test_text.size(); ++i) {
        auto color = g_disabled_color;
        if (m_first_row_headers->isChecked()) {
            color = g_header_color;
        } else if (m_role_selection[i].second->isChecked()) {
            color = g_enabled_color;
        } else {

        }

        auto w = m_preview_table->item(2, int(i));
        w->setBackgroundColor(color);
    }
}


const std::tuple<size_t, size_t, ssize_t, bool> CsvImporter::get_config() const
{
    if (!m_is_valid) {
        throw AppException("Requested invalid configuration");
    }

    return m_config;
}


void CsvImporter::attempt_autoconfig()
{
    auto headers_in_row = false;

    std::array<int, TestFields::HEADER_ROW> fields;
    std::fill(fields.begin(), fields.end(), -1);

    size_t row = 0U;
    auto re = QRegExp("\\d{1,5}");

    bool strings_in_row;
    int int8_in_row;
    std::vector<int> candidate_columns;

    do {
        strings_in_row = false;
        int8_in_row = -1;
        candidate_columns.clear();

        for (size_t i=0U; i<m_test_text.size(); ++i) {
            const auto &test = m_test_text[i][row];
            if (re.exactMatch(test)) {
                /* Test if contents are purely numerical */
                const auto test_int = test.toInt();
                if (test_int >= 0 && test_int <= 0xFFFF) {
                    candidate_columns.push_back(int(i));
                    if (test_int <= 0xFF) {
                        int8_in_row = int(i);
                    }
                }
            } else if (0 == row) {
                strings_in_row = true;

                /* Test for known column header titles */
                const auto test_lower = test.toLower();
                if (tr("Raw Value").toLower() == test_lower) {
                    //  Special case...
                    fields[TestFields::REG_VALUE] = int(i);
                    headers_in_row = true;
                }

                for (auto j=0; j<m_options.size(); ++j) {
                    if (m_options[j].toLower() == test_lower && fields[size_t(j)] < 0) {
                        headers_in_row = true;
                        fields[size_t(j)] = int(i);
                        break;
                    }
                }
            } else {
                strings_in_row = true;
            }
        }

        if (!strings_in_row) {
            break;
        }

        if (!headers_in_row && (candidate_columns.size() >= 2U)) {
            break;
        }
    } while (++row < 2);

    /* See if there's a good argument for assigning the node column */
    if (fields[TestFields::NODE_ID] < 0 && candidate_columns.size() > 2U && int8_in_row >= 0) {
        //  No known column already assigned as the node and we have at least 3
        // columns with only numbers, use the last one that has a value < 256
        fields[TestFields::NODE_ID] = int8_in_row;
    }

    /* Now, try to assign any remaining unassigned fields */
    std::unordered_set<int> used_fields;
    for (auto i: fields) {
        if (i >= 0) {
            used_fields.insert(i);
        }
    }
    for (auto i: candidate_columns) {
        if (used_fields.find(i) == used_fields.end()) {
            if (fields[TestFields::REG_NUMBER] < 0) {
                fields[TestFields::REG_NUMBER] = i;
            } else if (fields[TestFields::REG_VALUE] < 0) {
                fields[TestFields::REG_VALUE] = i;
            } else {
                break;
            }
        }
    }

    /* Update controls according to what has been discovered */
    if (headers_in_row || row > 0) {
        m_first_row_headers->setChecked(true);
    }

    if (fields[TestFields::NODE_ID] < 0) {
        m_fixed_node->setChecked(true);
    }

    for (auto i=0; i<m_options.size(); ++i) {
        const auto column = fields[size_t(i)];
        if (column >= 0) {
            m_role_selection[size_t(column)].second->setChecked(true);
            m_role_selection[size_t(column)].first->setCurrentIndex(i);
        }
    }
}
