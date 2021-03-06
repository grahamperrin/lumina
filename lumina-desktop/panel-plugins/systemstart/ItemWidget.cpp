//===========================================
//  Lumina-DE source code
//  Copyright (c) 2014-2015, Ken Moore
//  Available under the 3-clause BSD license
//  See the LICENSE file for full details
//===========================================
#include "ItemWidget.h"
#include <LuminaUtils.h>
#include <QMenu>

#define TEXTCUTOFF 165
ItemWidget::ItemWidget(QWidget *parent, QString itemPath, QString type, bool goback) : QFrame(parent){
  createWidget();
  //Now fill it appropriately
  bool inHome = type.endsWith("-home"); //internal code
  if(inHome){ type = type.remove("-home"); }
  if(itemPath.endsWith(".desktop") || type=="app"){
    bool ok = false;
    XDGDesktop item = LXDG::loadDesktopFile(itemPath, ok);
    if(ok && LXDG::checkValidity(item) ){
      icon->setPixmap( LXDG::findIcon(item.icon, "preferences-system-windows-actions").pixmap(32,32) );
      name->setText( this->fontMetrics().elidedText(item.name, Qt::ElideRight, TEXTCUTOFF) );
      setupActions(item);
    }else{
      gooditem = false;
      return;
    }
  }else if(type=="dir"){
    actButton->setVisible(false);
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    if(goback){
      icon->setPixmap( LXDG::findIcon("go-previous","").pixmap(32,32) );
      name->setText( tr("Go Back") );
    }else{
      icon->setPixmap( LXDG::findIcon("folder","").pixmap(32,32) );
      name->setText( this->fontMetrics().elidedText(itemPath.section("/",-1), Qt::ElideRight, TEXTCUTOFF) ); 
    }
  }else{
    actButton->setVisible(false);
    if(itemPath.endsWith("/")){ itemPath.chop(1); }
    if(QFileInfo(itemPath).isDir()){
      type = "dir";
      icon->setPixmap( LXDG::findIcon("folder","").pixmap(32,32) );
    }else if(LUtils::imageExtensions().contains(itemPath.section("/",-1).section(".",-1).toLower()) ){
      icon->setPixmap( QIcon(itemPath).pixmap(32,32) );
    }else{
      icon->setPixmap( LXDG::findMimeIcon(itemPath.section("/",-1)).pixmap(32,32) );
    }
    name->setText( this->fontMetrics().elidedText(itemPath.section("/",-1), Qt::ElideRight, TEXTCUTOFF) ); 
  }
  icon->setWhatsThis(itemPath);
  if(!goback){ this->setWhatsThis(name->text()); }
  isDirectory = (type=="dir"); //save this for later
  if(LUtils::isFavorite(itemPath)){
    linkPath = itemPath;
    isShortcut=true;
  }else if( inHome ){//|| itemPath.section("/",0,-2)==QDir::homePath()+"/Desktop" ){
    isShortcut = true;
  }else{
    isShortcut = false;
  }
  if(isShortcut){
    name->setToolTip(icon->whatsThis()); //also allow the user to see the full shortcut path
  }
  //Now setup the button appropriately
  setupContextMenu();
}

ItemWidget::ItemWidget(QWidget *parent, XDGDesktop item) : QFrame(parent){
  createWidget();
  isDirectory = false;
  if(LUtils::isFavorite(item.filePath)){
    linkPath = item.filePath;
    isShortcut=true;
  }else if( item.filePath.section("/",0,-2)==QDir::homePath()+"/Desktop" ){
    isShortcut = true;
  }else{
    isShortcut = false;
  }
  if(isShortcut){
    name->setToolTip(icon->whatsThis()); //also allow the user to see the full shortcut path
  }
  //Now fill it appropriately
  icon->setPixmap( LXDG::findIcon(item.icon,"preferences-system-windows-actions").pixmap(32,32) );
  name->setText( this->fontMetrics().elidedText(item.name, Qt::ElideRight, TEXTCUTOFF) ); 
  this->setWhatsThis(name->text());
  icon->setWhatsThis(item.filePath);
  //Now setup the buttons appropriately
  setupContextMenu();
  setupActions(item);
}

ItemWidget::~ItemWidget(){ 
}


void ItemWidget::createWidget(){
  //Initialize the widgets
  gooditem = true;
  menuopen = false;
  menureset = new QTimer(this);
    menureset->setSingleShot(true);
    menureset->setInterval(1000); //1 second	
  this->setContentsMargins(0,0,0,0);
  contextMenu = new QMenu();
    connect(contextMenu, SIGNAL(aboutToShow()), this, SLOT(actionMenuOpen()) );
    connect(contextMenu, SIGNAL(aboutToHide()), this, SLOT(actionMenuClosed()) );
  /*button = new QToolButton(this);
    button->setIconSize( QSize(14,14) );
    button->setAutoRaise(true);*/
  actButton = new QToolButton(this);
    actButton->setPopupMode(QToolButton::InstantPopup);
    actButton->setFixedSize( QSize(17,34) );
    actButton->setArrowType(Qt::DownArrow);
  icon = new QLabel(this);
    icon->setFixedSize( QSize(34,34) );
  name = new QLabel(this);
  //Add them to the layout
  this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(1,1,1,1);
    this->layout()->addWidget(icon);
    this->layout()->addWidget(actButton);
    this->layout()->addWidget(name);
  //Set a custom object name so this can be tied into the Lumina Theme stylesheets
  this->setObjectName("LuminaItemWidget");
}

void ItemWidget::setupContextMenu(){
  //Now refresh the context menu
  contextMenu->clear();
  if(!QFile::exists(QDir::homePath()+"/Desktop/"+icon->whatsThis().section("/",-1)) ){
    //Does not have a desktop link
    contextMenu->addAction( LXDG::findIcon("preferences-desktop-icons",""), tr("Pin to Desktop"), this, SLOT(PinToDesktop()) );
  }
  //Favorite Item
  if( LUtils::isFavorite(icon->whatsThis()) ){ //Favorite Item - can always remove this
    contextMenu->addAction( LXDG::findIcon("edit-delete",""), tr("Remove from Favorites"), this, SLOT(RemoveFavorite()) );
  }else{
    //This file does not have a shortcut yet -- allow the user to add it
    contextMenu->addAction( LXDG::findIcon("bookmark-toolbar",""), tr("Add to Favorites"), this, SLOT(AddFavorite()) );
  }
}

void ItemWidget::setupActions(XDGDesktop app){
  if(app.actions.isEmpty()){ actButton->setVisible(false); return; }
  //Actions Available - go ahead and list them all
  actButton->setMenu( new QMenu(this) );
  for(int i=0; i<app.actions.length(); i++){
    QAction *act = new QAction(LXDG::findIcon(app.actions[i].icon, app.icon), app.actions[i].name, this);
	act->setToolTip(app.actions[i].ID);
        act->setWhatsThis(app.actions[i].ID);
        actButton->menu()->addAction(act);	
  }
  connect(actButton->menu(), SIGNAL(triggered(QAction*)), this, SLOT(actionClicked(QAction*)) );
  connect(actButton->menu(), SIGNAL(aboutToShow()), this, SLOT(actionMenuOpen()) );
  connect(actButton->menu(), SIGNAL(aboutToHide()), this, SLOT(actionMenuClosed()) );
  connect(menureset, SIGNAL(timeout()), this, SLOT(resetmenuflag()) );
}

void ItemWidget::PinToDesktop(){
  qDebug() << "Create Link on Desktop:" << icon->whatsThis();
  bool ok = QFile::link(icon->whatsThis(), QDir::homePath()+"/Desktop/"+icon->whatsThis().section("/",-1));
  qDebug() << " - " << (ok ? "Success": "Failure");
}

void ItemWidget::RemoveFavorite(){
  LUtils::removeFavorite(icon->whatsThis());
  linkPath.clear();
  emit RemovedShortcut();
}

void ItemWidget::AddFavorite(){
  if( LUtils::addFavorite(icon->whatsThis()) ){
    linkPath = icon->whatsThis();
    emit NewShortcut();	
  }
  
}


void ItemWidget::ItemClicked(){
  if(!linkPath.isEmpty()){ emit RunItem(linkPath); }
  else{ emit RunItem(icon->whatsThis()); }
}

void ItemWidget::actionClicked(QAction *act){
  actButton->menu()->hide();
  QString cmd = "lumina-open -action \""+act->whatsThis()+"\" \"%1\"";
  if(!linkPath.isEmpty()){ cmd = cmd.arg(linkPath); }
  else{ cmd = cmd.arg(icon->whatsThis()); }
  emit RunItem(cmd);
}