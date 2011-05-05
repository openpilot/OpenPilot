/**
 ******************************************************************************
 *
 * @file       coreconstants.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef CORECONSTANTS_H
#define CORECONSTANTS_H

namespace Core {
namespace Constants {

#define GCS_VERSION_MAJOR 1
#define GCS_VERSION_MINOR 0
#define GCS_VERSION_RELEASE 0
const char * const GCS_VERSION_TYPE = "Alpha";
const char * const GCS_VERSION_CODENAME = "Pascal";

#define STRINGIFY_INTERNAL(x) #x
#define STRINGIFY(x) STRINGIFY_INTERNAL(x)

#define GCS_VERSION STRINGIFY(GCS_VERSION_MAJOR) \
    "." STRINGIFY(GCS_VERSION_MINOR) \
    "." STRINGIFY(GCS_VERSION_RELEASE)

const char * const GCS_VERSION_LONG      = GCS_VERSION;
const char * const GCS_AUTHOR            = "OpenPilot Project";
const char * const GCS_YEAR              = "2011";

#ifdef GCS_REVISION
const char * const GCS_REVISION_STR      = STRINGIFY(GCS_REVISION);
#else
const char * const GCS_REVISION_STR      = "";
#endif

#undef GCS_VERSION
#undef STRINGIFY
#undef STRINGIFY_INTERNAL

//modes
const char * const MODE_WELCOME          = "Welcome";
const char * const MODE_UAVGADGET        = "Mode 1";
const int          P_MODE_WELCOME        = 100;
const int          P_MODE_UAVGADGET      = 90;

//menubar
const char * const MENU_BAR              = "GCS.MenuBar";

//menus
const char * const M_FILE                = "GCS.Menu.File";
const char * const M_FILE_OPEN           = "GCS.Menu.File.Open";
const char * const M_FILE_NEW            = "GCS.Menu.File.New";
const char * const M_FILE_RECENTFILES    = "GCS.Menu.File.RecentFiles";
const char * const M_EDIT                = "GCS.Menu.Edit";
const char * const M_EDIT_ADVANCED       = "GCS.Menu.Edit.Advanced";
const char * const M_TOOLS               = "GCS.Menu.Tools";
const char * const M_WINDOW              = "GCS.Menu.Window";
const char * const M_WINDOW_PANES        = "GCS.Menu.Window.Panes";
const char * const M_HELP                = "GCS.Menu.Help";

//contexts
const char * const C_GLOBAL              = "Global Context";
const int          C_GLOBAL_ID           = 0;
const char * const C_WELCOME_MODE        = "Core.WelcomeMode";
const char * const C_UAVGADGET_MODE      = "Core.UAVGadgetMode";
const char * const C_UAVGADGETMANAGER    = "Core.UAVGadgetManager";
const char * const C_NAVIGATION_PANE     = "Core.NavigationPane";
const char * const C_PROBLEM_PANE        = "Core.ProblemPane";

//default editor kind
const char * const K_DEFAULT_TEXT_EDITOR = QT_TRANSLATE_NOOP("OpenWith::Editors", "Plain Text Editor");
const char * const K_DEFAULT_BINARY_EDITOR = QT_TRANSLATE_NOOP("OpenWith::Editors", "Binary Editor");

//actions
const char * const UNDO                  = "GCS.Undo";
const char * const REDO                  = "GCS.Redo";
const char * const COPY                  = "GCS.Copy";
const char * const PASTE                 = "GCS.Paste";
const char * const CUT                   = "GCS.Cut";
const char * const SELECTALL             = "GCS.SelectAll";

const char * const NEW                   = "GCS.New";
const char * const OPEN                  = "GCS.Open";
const char * const OPEN_WITH             = "GCS.OpenWith";
const char * const REVERTTOSAVED         = "GCS.RevertToSaved";
const char * const SAVE                  = "GCS.Save";
const char * const SAVEAS                = "GCS.SaveAs";
const char * const SAVEALL               = "GCS.SaveAll";
const char * const EXIT                  = "GCS.Exit";

const char * const OPTIONS               = "GCS.Options";
const char * const TOGGLE_SIDEBAR        = "GCS.ToggleSidebar";
const char * const TOGGLE_FULLSCREEN     = "GCS.ToggleFullScreen";

const char * const MINIMIZE_WINDOW       = "GCS.MinimizeWindow";
const char * const ZOOM_WINDOW           = "GCS.ZoomWindow";

const char * const SPLIT                 = "GCS.Split";
const char * const SPLIT_SIDE_BY_SIDE    = "GCS.SplitSideBySide";
const char * const REMOVE_CURRENT_SPLIT  = "GCS.RemoveCurrentSplit";
const char * const REMOVE_ALL_SPLITS     = "GCS.RemoveAllSplits";
const char * const GOTO_OTHER_SPLIT      = "GCS.GotoOtherSplit";
const char * const SAVEASDEFAULT         = "GCS.SaveAsDefaultLayout";
const char * const RESTOREDEFAULT        = "GCS.RestoreDefaultLayout";
const char * const HIDE_TOOLBARS         = "GCS.HideToolbars";
const char * const CLOSE                 = "GCS.Close";
const char * const CLOSEALL              = "GCS.CloseAll";
const char * const CLOSEOTHERS           = "GCS.CloseOthers";
const char * const GOTONEXT              = "GCS.GotoNext";
const char * const GOTOPREV              = "GCS.GotoPrevious";
const char * const GOTONEXTINHISTORY     = "GCS.GotoNextInHistory";
const char * const GOTOPREVINHISTORY     = "GCS.GotoPreviousInHistory";
const char * const GO_BACK               = "GCS.GoBack";
const char * const GO_FORWARD            = "GCS.GoForward";
const char * const GOTOPREVIOUSGROUP     = "GCS.GotoPreviousTabGroup";
const char * const GOTONEXTGROUP         = "GCS.GotoNextTabGroup";
const char * const WINDOWSLIST           = "GCS.WindowsList";
const char * const ABOUT_OPENPILOTGCS    = "GCS.AboutOpenPilotGCS";
const char * const ABOUT_PLUGINS         = "GCS.AboutPlugins";
const char * const ABOUT_AUTHORS         = "GCS.AboutAuthors";
const char * const ABOUT_QT              = "GCS.AboutQt";
const char * const S_RETURNTOEDITOR      = "GCS.ReturnToEditor";
const char * const OPEN_IN_EXTERNAL_EDITOR = "GCS.OpenInExternalEditor";

// default groups
const char * const G_DEFAULT_ONE         = "GCS.Group.Default.One";
const char * const G_DEFAULT_TWO         = "GCS.Group.Default.Two";
const char * const G_DEFAULT_THREE       = "GCS.Group.Default.Three";

// main menu bar groups
const char * const G_FILE                = "GCS.Group.File";
const char * const G_EDIT                = "GCS.Group.Edit";
const char * const G_VIEW                = "GCS.Group.View";
const char * const G_TOOLS               = "GCS.Group.Tools";
const char * const G_WINDOW              = "GCS.Group.Window";
const char * const G_HELP                = "GCS.Group.Help";

// file menu groups
const char * const G_FILE_NEW            = "GCS.Group.File.New";
const char * const G_FILE_OPEN           = "GCS.Group.File.Open";
const char * const G_FILE_PROJECT        = "GCS.Group.File.Project";
const char * const G_FILE_SAVE           = "GCS.Group.File.Save";
const char * const G_FILE_CLOSE          = "GCS.Group.File.Close";
const char * const G_FILE_OTHER          = "GCS.Group.File.Other";

// edit menu groups
const char * const G_EDIT_UNDOREDO       = "GCS.Group.Edit.UndoRedo";
const char * const G_EDIT_COPYPASTE      = "GCS.Group.Edit.CopyPaste";
const char * const G_EDIT_SELECTALL      = "GCS.Group.Edit.SelectAll";
const char * const G_EDIT_ADVANCED       = "GCS.Group.Edit.Advanced";

const char * const G_EDIT_FIND           = "GCS.Group.Edit.Find";
const char * const G_EDIT_OTHER          = "GCS.Group.Edit.Other";

// advanced edit menu groups

const char * const G_EDIT_FORMAT         = "GCS.Group.Edit.Format";
const char * const G_EDIT_COLLAPSING     = "GCS.Group.Edit.Collapsing";
const char * const G_EDIT_BLOCKS         = "GCS.Group.Edit.Blocks";
const char * const G_EDIT_FONT           = "GCS.Group.Edit.Font";
const char * const G_EDIT_EDITOR         = "GCS.Group.Edit.Editor";

// window menu groups
const char * const G_WINDOW_SIZE         = "GCS.Group.Window.Size";
const char * const G_WINDOW_PANES        = "GCS.Group.Window.Panes";
const char * const G_WINDOW_SPLIT        = "GCS.Group.Window.Split";
const char * const G_WINDOW_NAVIGATE     = "GCS.Group.Window.Navigate";
const char * const G_WINDOW_OTHER        = "GCS.Group.Window.Other";
const char * const G_WINDOW_HIDE_TOOLBAR = "GCS.Group.Window.Hide";

// help groups (global)
const char * const G_HELP_HELP           = "GCS.Group.Help.Help";
const char * const G_HELP_ABOUT          = "GCS.Group.Help.About";

const char * const ICON_MINUS            = ":/core/images/minus.png";
const char * const ICON_PLUS             = ":/core/images/plus.png";
const char * const ICON_NEWFILE          = ":/core/images/filenew.png";
const char * const ICON_OPENFILE         = ":/core/images/fileopen.png";
const char * const ICON_SAVEFILE         = ":/core/images/filesave.png";
const char * const ICON_UNDO             = ":/core/images/undo.png";
const char * const ICON_REDO             = ":/core/images/redo.png";
const char * const ICON_COPY             = ":/core/images/editcopy.png";
const char * const ICON_PASTE            = ":/core/images/editpaste.png";
const char * const ICON_CUT              = ":/core/images/editcut.png";
const char * const ICON_NEXT             = ":/core/images/next.png";
const char * const ICON_PREV             = ":/core/images/prev.png";
const char * const ICON_DIR              = ":/core/images/dir.png";
const char * const ICON_CLEAN_PANE       = ":/core/images/clean_pane_small.png";
const char * const ICON_CLEAR            = ":/core/images/clear.png";
const char * const ICON_FIND             = ":/core/images/find.png";
const char * const ICON_FINDNEXT         = ":/core/images/findnext.png";
const char * const ICON_REPLACE          = ":/core/images/replace.png";
const char * const ICON_RESET            = ":/core/images/reset.png";
const char * const ICON_MAGNIFIER        = ":/core/images/magnifier.png";
const char * const ICON_TOGGLE_SIDEBAR   = ":/core/images/sidebaricon.png";
const char * const ICON_PLUGIN           = ":/core/images/pluginicon.png";
const char * const ICON_EXIT             = ":/core/images/exiticon.png";
const char * const ICON_OPTIONS          = ":/core/images/optionsicon.png";
const char * const ICON_HELP             = ":/core/images/helpicon.png";
const char * const ICON_OPENPILOT        = ":/core/images/openpiloticon.png";


// wizard kind
const char * const WIZARD_TYPE_FILE      = "GCS::WizardType::File";
const char * const WIZARD_TYPE_CLASS     = "GCS::WizardType::Class";

} // namespace Constants
} // namespace Core

#endif // CORECONSTANTS_H
