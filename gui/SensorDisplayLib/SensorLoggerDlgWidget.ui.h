/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void SensorLoggerDlgWidget::init()
{
  m_lowerLimit->setValidator(new KDoubleValidator(m_lowerLimit));
  m_upperLimit->setValidator(new KDoubleValidator(m_upperLimit));
  m_timerInterval->setRange(1, 99, 1, true);

  m_fileName->setFocus();
}


QString SensorLoggerDlgWidget::fileName()
{
    return m_fileName->url();
}


int SensorLoggerDlgWidget::timerInterval()
{
    return m_timerInterval->value();
}


bool SensorLoggerDlgWidget::lowerLimitActive()
{
    return m_lowerLimitActive->isChecked();
}


double SensorLoggerDlgWidget::lowerLimit()
{
    return m_lowerLimit->text().toDouble();
}


bool SensorLoggerDlgWidget::upperLimitActive()
{
    return m_upperLimitActive->isChecked();
}


double SensorLoggerDlgWidget::upperLimit()
{
    return m_upperLimit->text().toDouble();
}


void SensorLoggerDlgWidget::setFileName( const QString &url )
{
    m_fileName->setUrl(url);
}


void SensorLoggerDlgWidget::setTimerInterval( int i )
{
    m_timerInterval->setValue(i);
}


void SensorLoggerDlgWidget::setLowerLimitActive( bool b )
{
    m_lowerLimitActive->setChecked(b);
}


void SensorLoggerDlgWidget::setLowerLimit( double d )
{
    m_lowerLimit->setText(QString("%1").arg(d));
}


void SensorLoggerDlgWidget::setUpperLimitActive( bool b )
{
    m_upperLimitActive->setChecked(b);
}


void SensorLoggerDlgWidget::setUpperLimit( double d )
{
    m_upperLimit->setText(QString("%1").arg(d));
}
