/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */


class ValueSlider : public Frame
{
public:
  ValueSlider(int min, int max, int maxmax, const Glib::ustring& label_text);

  inline int getValue(void) { return value; }
  void setValue(int val);
  
private:
  HBox hBox;
  Adjustment adjustment;
  HScale scale;
  Entry entry;

  int value;
  double maxmax;

public:
  SigC::Signal0<void> signal_valueChanged();

private:
  SigC::Signal0<void> m_signal_valueChanged;

  bool setScaleToEntry(void);

  bool setEntryToScale(void);

  void on_adjustmentValueChanged(void);
  void on_entryValueChanged(void);

  bool inSettingValues;
};


class DoubleValueSlider : public Frame
{
public:
  DoubleValueSlider(double min, double max, double maxmax, const Glib::ustring& label_text);

  inline double getValue(void) { return value; }
  void setValue(double val);
  
private:
  HBox hBox;
  Adjustment adjustment;
  HScale scale;
  Entry entry;

  double value, maxmax;

public:
  SigC::Signal0<void> signal_valueChanged();

private:
  SigC::Signal0<void> m_signal_valueChanged;

  bool setScaleToEntry(void);

  bool setEntryToScale(void);

  void on_adjustmentValueChanged(void);
  void on_entryValueChanged(void);

  bool inSettingValues;
};


// Like ValueSlider with a logarithmically changing scale.
class LogValueSlider : public Frame
{
public:
  LogValueSlider(int min, int max, const Glib::ustring& label_text);

  int getValue(void);
  void setValue(int val);
  
private:
  HBox hBox;
  Adjustment adjustment;
  HScale scale;
  Entry entry;

  // log of getValue()
  double value;

public:
  SigC::Signal0<void> signal_valueChanged();

private:
  SigC::Signal0<void> m_signal_valueChanged;

  bool setScaleToEntry(void);

  bool setEntryToScale(void);

  void on_adjustmentValueChanged(void);
  void on_entryValueChanged(void);

  bool inSettingValues;
};

