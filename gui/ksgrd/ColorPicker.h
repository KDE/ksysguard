/*
    KSysGuard, the KDE System Guard
   
    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    KSysGuard is currently maintained by Chris Schlaeger <cs@kde.org>.
    Please do not commit any changes without consulting me first. Thanks!

    $Id$
*/

#ifndef KSG_COLORPICKER_H
#define KSG_COLORPICKER_H

class QFrame;
class QHBoxLayout;
class QLabel;
class QPushButton;

class ColorPicker : public QWidget
{
	Q_OBJECT
	Q_PROPERTY( QString text READ text WRITE setText )
	Q_PROPERTY( QColor color READ color WRITE setColor )

  public:
    ColorPicker( QWidget *parent, const char *name = 0 );
    ~ColorPicker();

    void setText( const QString &text );
    QString text() const;

    void setColor( const QColor &color );
    QColor color() const;

  private slots:
    void raiseColorDialog();

  private:
    QFrame* mColorBox;
    QLabel* mText;
    QPushButton* mButton;
};

#endif
