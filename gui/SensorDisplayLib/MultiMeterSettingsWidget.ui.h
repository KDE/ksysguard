/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void MultiMeterSettingsWidget::init()
{
  m_lowerLimit->setValidator(new KDoubleValidator(m_lowerLimit));
  m_upperLimit->setValidator(new KDoubleValidator(m_upperLimit));

  m_title->setFocus();
}

QString MultiMeterSettingsWidget::title()
{
    return m_title->text();
}


bool MultiMeterSettingsWidget::showUnit()
{
    return m_showUnit->isChecked();
}


bool MultiMeterSettingsWidget::lowerLimitActive()
{
    return m_lowerLimitActive->isChecked();
}


double MultiMeterSettingsWidget::lowerLimit()
{
    return m_lowerLimit->text().toDouble();
}


bool MultiMeterSettingsWidget::upperLimitActive()
{
    return m_upperLimitActive->isChecked();
}


double MultiMeterSettingsWidget::upperLimit()
{
    return m_upperLimit->text().toDouble();
}


QColor MultiMeterSettingsWidget::normalDigitColor()
{
    return m_normalDigitColor->color();
}


QColor MultiMeterSettingsWidget::alarmDigitColor()
{
    return m_alarmDigitColor->color();
}


QColor MultiMeterSettingsWidget::meterBackgroundColor()
{
    return m_backgroundColor->color();
}


void MultiMeterSettingsWidget::setTitle( const QString &s )
{
    m_title->setText(s);
}


void MultiMeterSettingsWidget::setShowUnit( bool b )
{
    m_showUnit->setChecked(b);
}


void MultiMeterSettingsWidget::setLowerLimitActive( bool b )
{
    m_lowerLimitActive->setChecked(b);
}


void MultiMeterSettingsWidget::setLowerLimit( double d )
{
    m_lowerLimit->setText(QString("%1").arg(d));
}


void MultiMeterSettingsWidget::setUpperLimitActive( bool b )
{
    m_upperLimitActive->setChecked(b);
}


void MultiMeterSettingsWidget::setUpperLimit( double d )
{
    m_upperLimit->setText(QString("%1").arg(d));
}


void MultiMeterSettingsWidget::setNormalDigitColor( const QColor &c )
{
    m_normalDigitColor->setColor(c);
}


void MultiMeterSettingsWidget::setAlarmDigitColor( const QColor &c )
{
    m_alarmDigitColor->setColor(c);
}


void MultiMeterSettingsWidget::setMeterBackgroundColor( const QColor &c )
{
    m_backgroundColor->setColor(c);
}
