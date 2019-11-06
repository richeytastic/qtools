/************************************************************************
 * Copyright (C) 2019 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef QTOOLS_TREE_COMBO_BOX_H
#define QTOOLS_TREE_COMBO_BOX_H

#include "QTools_Export.h"    // QTools
#include <QComboBox>

namespace QTools {

class QTools_EXPORT TreeComboBox : public QComboBox
{ Q_OBJECT
public:
    explicit TreeComboBox( QWidget* parent=nullptr);

protected:
    bool eventFilter( QObject*, QEvent*) override;
    void showPopup() override;
    void hidePopup() override;

private:
    bool _skipNextHide;
};  // end class

}   // end namespace

#endif
