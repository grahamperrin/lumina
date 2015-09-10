//===========================================
//  Lumina-DE source code
//  Copyright (c) 2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
//  This panel plugin is the main button that allow the user to run 
//    applications or logout of the desktop
//===========================================
#ifndef _LUMINA_DESKTOP_START_MENU_PLUGIN_H
#define _LUMINA_DESKTOP_START_MENU_PLUGIN_H

// Qt includes
#include <QMenu>
#include <QWidgetAction>
#include <QToolButton>
#include <QString>
#include <QWidget>


// Lumina-desktop includes
//#include "../../Globals.h"
#include "../LPPlugin.h" //main plugin widget

// libLumina includes
#include "LuminaXDG.h"

#include "StartMenu.h"

// PANEL PLUGIN BUTTON
class LStartButtonPlugin : public LPPlugin{
	Q_OBJECT
	
public:
	LStartButtonPlugin(QWidget *parent = 0, QString id = "systemstart", bool horizontal=true);
	~LStartButtonPlugin();
	
private:
	QMenu *menu;
	QWidgetAction *mact;
	StartMenu *startmenu;
	QToolButton *button;

private slots:
	void openMenu();
	void closeMenu();

	void updateButtonVisuals();

public slots:
	void OrientationChange(){
	  if(this->layout()->direction()==QBoxLayout::LeftToRight){
	    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
	    button->setIconSize( QSize(this->height(), this->height()) );
	  }else{
	    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	    button->setIconSize( QSize(this->width(), this->width()) );
	  }
	  this->layout()->update();
	  updateButtonVisuals();
	}
	
	void LocaleChange(){ 
	  updateButtonVisuals();
	  startmenu->UpdateAll();
	}
	
	void ThemeChange(){
	  updateButtonVisuals();
	  startmenu->UpdateAll();
	}
};

#endif