/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include <gtkmm.h>
#include <glibmm/ustring.h>
#include <gtkmm/label.h>

using namespace Gtk;
#include "ValueSlider.h"



ValueSlider::ValueSlider(int min, int max, int maxmax_in,
                         const Glib::ustring& label_text):
  adjustment((double)min, (double)min, (double)max, 1.0, 1.0),
  scale(adjustment)
{
  maxmax = maxmax_in;
  set_label(label_text);
  hBox.set_spacing(6);
  add(hBox);
  hBox.pack_start(scale, PACK_EXPAND_WIDGET);
  hBox.pack_start(entry, PACK_SHRINK);

  scale.set_size_request(100, -1);
  scale.set_draw_value(false);
  scale.set_digits(10);
  entry.set_size_request(40, -1);
  entry.set_max_length(32);
  value = min;
  setScaleToEntry();
  entry.set_editable();

  inSettingValues = false;
    
  adjustment.signal_value_changed().
    connect( sigc::mem_fun(*this, &ValueSlider::on_adjustmentValueChanged));
  
  entry.signal_activate().
    connect( sigc::mem_fun(*this, &ValueSlider::on_entryValueChanged));
}

void ValueSlider::setValue(int val)
{
  double dval = val;
  
  if(dval<adjustment.get_lower())
    dval = adjustment.get_lower();
  else if(val>adjustment.get_upper())
  {
    if(val>maxmax)
      val = (int) maxmax;
    if(val>adjustment.get_upper())
      adjustment.set_upper(val);
  }


  if(value != (int) dval)
  {
    value = (int) dval;
    dval = value;
    // this time we let it emit a value change signal.
    adjustment.set_value(dval);
    setScaleToEntry();
  }
}

void ValueSlider::on_adjustmentValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setScaleToEntry())
      m_signal_valueChanged.emit();
  }
}

void ValueSlider::on_entryValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setEntryToScale())
      m_signal_valueChanged.emit();
  }
}

bool ValueSlider::setScaleToEntry(void)
{
  double dval = adjustment.get_value();
  int ival = (int) dval;
  
  if(dval != (double) ival)
  {
    inSettingValues = true;
    adjustment.set_value((double) ival);
    inSettingValues = false;
  }
  
  char str[32];
  snprintf(str,32, "%d", ival);
  inSettingValues = true;
  entry.set_text(str);
  inSettingValues = false;
  
  if(ival == value)
    return false; // no value change
  else
  {
    value = ival;
    return true;
  }
}

bool ValueSlider::setEntryToScale(void)
{
  Glib::ustring str = entry.get_text();
  errno = 0;
  char *ptr = (char *) str.c_str();
  if(!ptr) return false;

  double val_read = strtod(str.c_str(), &ptr);
  
  if(ptr==(char *) str.c_str() || errno)
  {
    setScaleToEntry();
    return false; // no value change
  }
  else
  {
    double val = val_read;
    
    if(val<adjustment.get_lower())
      val = adjustment.get_lower();
    else if(val>adjustment.get_upper())
    {
      if(val>maxmax)
        val = maxmax;
      if(val>adjustment.get_upper())
        adjustment.set_upper(val);
    }

    if(val != val_read)
    {
      char str[32];
      snprintf(str,32, "%d", (int) val);
      inSettingValues = true;
      entry.set_text(str);
      inSettingValues = false;
    }
    
    entry.select_region(0, entry.get_text_length());
    
    if(value != (int) val)
    {
      value = (int) val;
      val = value;
      
      inSettingValues = true;
      adjustment.set_value(val);
      inSettingValues = false;

      
      return true;
    }
    else
      return false; // no value change
  }
}

sigc::signal0<void> ValueSlider::signal_valueChanged()
{
  return m_signal_valueChanged;
}






DoubleValueSlider::DoubleValueSlider(double min, double max, double maxmax_in,
                                     const Glib::ustring& label_text):
  adjustment(min, min, max, (max-min)/1000, (max-min)/1000),
  scale(adjustment)
{
  maxmax = maxmax_in;
  set_label(label_text);
  hBox.set_spacing(6);
  add(hBox);
  hBox.pack_start(scale, PACK_EXPAND_WIDGET);
  hBox.pack_start(entry, PACK_SHRINK);

  scale.set_size_request(100, -1);
  scale.set_draw_value(false);
  scale.set_digits(10);
  entry.set_size_request(40, -1);
  entry.set_max_length(32);
  value = min;
  setScaleToEntry();
  entry.set_editable();

  inSettingValues = false;
    
  adjustment.signal_value_changed().
    connect( sigc::mem_fun(*this, &DoubleValueSlider::on_adjustmentValueChanged));
  
  entry.signal_activate().
    connect( sigc::mem_fun(*this, &DoubleValueSlider::on_entryValueChanged));
}

void DoubleValueSlider::setValue(double val)
{  
  if(val<adjustment.get_lower())
    val = adjustment.get_lower();
  else if(val>adjustment.get_upper())
  {
    if(val>maxmax)
      val = maxmax;
    if(val>adjustment.get_upper())
      adjustment.set_upper(val);
  }


  if(value != val)
  {
    value = val;
    // this time we let it emit a value change signal.
    adjustment.set_value(val);
    setScaleToEntry();
  }
}

void DoubleValueSlider::on_adjustmentValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setScaleToEntry())
      m_signal_valueChanged.emit();
  }
}

void DoubleValueSlider::on_entryValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setEntryToScale())
      m_signal_valueChanged.emit();
  }
}

bool DoubleValueSlider::setScaleToEntry(void)
{
  double val = adjustment.get_value();
  
  char str[32];
  snprintf(str,32, "%g", val);
  inSettingValues = true;
  entry.set_text(str);
  inSettingValues = false;

  if(value != val)
  {
    value = val;
    return true;
  }
  else
    return false;
}

bool DoubleValueSlider::setEntryToScale(void)
{
  Glib::ustring str = entry.get_text();
  errno = 0;
  char *ptr = (char *) str.c_str();
  if(!ptr) return false;

  double val_read = strtod(str.c_str(), &ptr);
  
  if(ptr==(char *) str.c_str() || errno)
  {
    setScaleToEntry();
    return false; // no value change
  }
  else
  {
    double val = val_read;
    
    if(val<adjustment.get_lower())
      val = adjustment.get_lower();
    else if(val>adjustment.get_upper())
    {
      if(val>maxmax)
        val = maxmax;
      if(val>adjustment.get_upper())
        adjustment.set_upper(val);
    }

    if(val != val_read)
    {
      char str[32];
      snprintf(str,32, "%d", (int) val);
      inSettingValues = true;
      entry.set_text(str);
      inSettingValues = false;
    }
    entry.select_region(0, entry.get_text_length());
    
    if(value != val)
    {
      val = value;
      
      inSettingValues = true;
      adjustment.set_value(val);
      inSettingValues = false;
      
      return true;
    }
    else
      return false; // no value change
  }
}

sigc::signal0<void> DoubleValueSlider::signal_valueChanged()
{
  return m_signal_valueChanged;
}






LogValueSlider::LogValueSlider(int min, int max, const Glib::ustring& label_text):
  adjustment(log10(min), log10(min),
             log10(max), 1.0, 1.0),
  scale(adjustment)
{
  set_label(label_text);
  hBox.set_spacing(6);
  add(hBox);
  hBox.pack_start(scale, PACK_EXPAND_WIDGET);
  hBox.pack_start(entry, PACK_SHRINK);

  scale.set_size_request(100, -1);
  scale.set_draw_value(false);
  scale.set_digits(10);
  entry.set_size_request(40, -1);
  entry.set_max_length(32);
  value = min;
  setScaleToEntry();
  entry.set_editable();
  
  inSettingValues = false;
    
  adjustment.signal_value_changed().
    connect( sigc::mem_fun(*this, &LogValueSlider::on_adjustmentValueChanged));
  
  entry.signal_activate().
    connect( sigc::mem_fun(*this, &LogValueSlider::on_entryValueChanged));
}

void LogValueSlider::setValue(int val)
{
  double dval = log10(val);
  
  if(dval<adjustment.get_lower())
    dval = adjustment.get_lower();
  else if(dval>adjustment.get_upper())
    dval = adjustment.get_upper();

  if(value != dval)
  {
    value = dval;
    // this time we let it emit a value change signal.
    adjustment.set_value(dval);
    setScaleToEntry();
  }
}

void LogValueSlider::on_adjustmentValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setScaleToEntry())
      m_signal_valueChanged.emit();
  }
}

void LogValueSlider::on_entryValueChanged(void)
{
  // This if() stops the infinite recursion of ValueChanged signals
  if(!inSettingValues)
  {
    if(setEntryToScale())
      m_signal_valueChanged.emit();
  }
}

bool LogValueSlider::setScaleToEntry(void)
{
  double dval = adjustment.get_value();
  
  char str[32];
  snprintf(str,32, "%d", (int) pow(10.0, dval));
  inSettingValues = true;
  entry.set_text(str);
  inSettingValues = false;
  
  if(dval == value)
    return false; // no value change
  else
  {
    value = dval;
    return true;
  }
}

bool LogValueSlider::setEntryToScale(void)
{
  Glib::ustring str = entry.get_text();
  errno = 0;
  char *ptr = (char *) str.c_str();
  if(!ptr) return false;

  double val_read = strtod(str.c_str(), &ptr);
  
  if(ptr==(char *) str.c_str() || errno)
  {
    setScaleToEntry();
    return false; // no value change
  }
  else
  {
    val_read = log10(val_read);
    double val = val_read;
    
    if(val<adjustment.get_lower())
      val = adjustment.get_lower();
    else if(val>adjustment.get_upper())
      val = adjustment.get_upper();

    if(val != val_read)
    {
      char str[32];
      snprintf(str,32, "%d", (int) pow(10.0, val));
      inSettingValues = true;
      entry.set_text(str);
      inSettingValues = false;
    }
    entry.select_region(0, entry.get_text_length());
    
    if(value != val)
    {
      value = val;
      
      inSettingValues = true;
      adjustment.set_value(val);
      inSettingValues = false;
      
      return true;
    }
    else
      return false; // no value change
  }
}

int LogValueSlider::getValue(void)
{
  return (int) pow(10.0, value);
}

sigc::signal0<void> LogValueSlider::signal_valueChanged()
{
  return m_signal_valueChanged;
}

