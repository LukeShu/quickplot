/* Copyright (c) 1998, 1999, 2003, 2004  Lance Arsenault, (GNU GPL (v2+))
 */

class FancyButton : public Button
{
public:
  FancyButton(const Glib::ustring& label, bool mnemonic=false);
};


class ButtonBar : public HButtonBox
{
public:

  ButtonBar(MainWindow *mainWindow_in);
  
  void checkGraphConfigButton(void);
  void on_showGraphConfigButton(void);
  
  FancyButton openButton, newButton,
    showGraphConfigButton, savePNGButton;

private:

  MainWindow *mainWindow;
};

