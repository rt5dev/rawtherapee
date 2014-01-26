/*
 *  This file is part of RawTherapee.
 *
 *  Copyright (c) 2004-2010 Gabor Horvath <hgabor@rawtherapee.com>
 *
 *  RawTherapee is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  RawTherapee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with RawTherapee.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "clipboard.h"
#include <gtkmm.h>
#include <fstream>
#include <string>
#include "guiutils.h"
#include "multilangmgr.h"
#include "guiutils.h"
#include "mycurve.h"
#include "shcselector.h"
#include "adjuster.h"
#include "mycurve.h"
#include "curveeditor.h"
#include "diagonalcurveeditorsubgroup.h"

DiagonalCurveEditorSubGroup::DiagonalCurveEditorSubGroup (CurveEditorGroup* prt, Glib::ustring& curveDir) : CurveEditorSubGroup(curveDir) {

	valLinear = (int)DCT_Linear;
	valUnchanged = (int)DCT_Unchanged;
	parent = prt;

	activeParamControl = -1;

	// custom curve
	customCurveBox = new Gtk::VBox ();
	customCurveBox->set_spacing(4);
	customCurve = Gtk::manage (new MyDiagonalCurve ());
	customCurve->set_size_request (GRAPH_SIZE+2*RADIUS, GRAPH_SIZE+2*RADIUS);
	customCurve->setType (DCT_Spline);
	//customCurve->set_tooltip_text (M("CURVEEDITOR_TOOLTIPMOVESPEED"));
	customCurveBox->pack_start (*customCurve, Gtk::PACK_EXPAND_WIDGET, 0);

	Gtk::HBox* custombbox = Gtk::manage (new Gtk::HBox ());
	custombbox->set_spacing(4);

	pasteCustom = Gtk::manage (new Gtk::Button ());
	pasteCustom->add (*Gtk::manage (new RTImage ("edit-paste.png")));
	copyCustom = Gtk::manage (new Gtk::Button ());
	copyCustom->add (*Gtk::manage (new RTImage ("edit-copy.png")));
	saveCustom = Gtk::manage (new Gtk::Button ());
	saveCustom->add (*Gtk::manage (new RTImage ("gtk-save-large.png")));
	loadCustom = Gtk::manage (new Gtk::Button ());
	loadCustom->add (*Gtk::manage (new RTImage ("gtk-open.png")));
	editCustom = Gtk::manage (new Gtk::ToggleButton());
	editCustom->add (*Gtk::manage (new RTImage ("editmodehand.png")));
	editCustom->hide();
	custombbox->pack_end (*pasteCustom, Gtk::PACK_SHRINK, 0);
	custombbox->pack_end (*copyCustom, Gtk::PACK_SHRINK, 0);
	custombbox->pack_end (*saveCustom, Gtk::PACK_SHRINK, 0);
	custombbox->pack_end (*loadCustom, Gtk::PACK_SHRINK, 0);
	custombbox->pack_start(*editCustom, Gtk::PACK_SHRINK, 0);

	customCurveBox->pack_end (*custombbox, Gtk::PACK_SHRINK, 0);
	customCurveBox->show_all ();

	saveCustom->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::savePressed) );
	loadCustom->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::loadPressed) );
	copyCustom->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::copyPressed) );
	pasteCustom->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::pastePressed) );
	editCustomConn = editCustom->signal_toggled().connect( sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::editToggled), editCustom) );

	saveCustom->set_tooltip_text (M("CURVEEDITOR_TOOLTIPSAVE"));
	loadCustom->set_tooltip_text (M("CURVEEDITOR_TOOLTIPLOAD"));
	copyCustom->set_tooltip_text (M("CURVEEDITOR_TOOLTIPCOPY"));
	pasteCustom->set_tooltip_text (M("CURVEEDITOR_TOOLTIPPASTE"));
	// Custom curve end


	// NURBS curve
	NURBSCurveBox = new Gtk::VBox ();
	NURBSCurveBox->set_spacing(4);
	NURBSCurve = Gtk::manage (new MyDiagonalCurve ());
	NURBSCurve->set_size_request (GRAPH_SIZE+2*RADIUS, GRAPH_SIZE+2*RADIUS);
	NURBSCurve->setType (DCT_NURBS);

	//customCurve->set_tooltip_text (M("CURVEEDITOR_TOOLTIPMOVESPEED"));
	NURBSCurveBox->pack_start (*NURBSCurve, Gtk::PACK_EXPAND_WIDGET, 0);

	Gtk::HBox* NURBSbbox = Gtk::manage (new Gtk::HBox ());
	NURBSbbox->set_spacing(4);
	pasteNURBS = Gtk::manage (new Gtk::Button ());
	pasteNURBS->add (*Gtk::manage (new RTImage ("edit-paste.png")));
	copyNURBS = Gtk::manage (new Gtk::Button ());
	copyNURBS->add (*Gtk::manage (new RTImage ("edit-copy.png")));
	saveNURBS = Gtk::manage (new Gtk::Button ());
	saveNURBS->add (*Gtk::manage (new RTImage ("gtk-save-large.png")));
	loadNURBS = Gtk::manage (new Gtk::Button ());
	loadNURBS->add (*Gtk::manage (new RTImage ("gtk-open.png")));
	editNURBS = Gtk::manage (new Gtk::ToggleButton());
	editNURBS->add (*Gtk::manage (new RTImage ("editmodehand.png")));
	editNURBS->hide();
	NURBSbbox->pack_end (*pasteNURBS, Gtk::PACK_SHRINK, 0);
	NURBSbbox->pack_end (*copyNURBS, Gtk::PACK_SHRINK, 0);
	NURBSbbox->pack_end (*saveNURBS, Gtk::PACK_SHRINK, 0);
	NURBSbbox->pack_end (*loadNURBS, Gtk::PACK_SHRINK, 0);
	NURBSbbox->pack_start(*editNURBS, Gtk::PACK_SHRINK, 0);

	NURBSCurveBox->pack_end (*NURBSbbox, Gtk::PACK_SHRINK, 0);
	NURBSCurveBox->show_all ();

	saveNURBS->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::savePressed) );
	loadNURBS->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::loadPressed) );
	pasteNURBS->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::pastePressed) );
	copyNURBS->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::copyPressed) );
	editNURBSConn = editNURBS->signal_toggled().connect( sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::editToggled), editNURBS) );

	saveNURBS->set_tooltip_text (M("CURVEEDITOR_TOOLTIPSAVE"));
	loadNURBS->set_tooltip_text (M("CURVEEDITOR_TOOLTIPLOAD"));
	pasteNURBS->set_tooltip_text (M("CURVEEDITOR_TOOLTIPPASTE"));
	copyNURBS->set_tooltip_text (M("CURVEEDITOR_TOOLTIPCOPY"));
	// NURBS curve end


	// parametric curve
	paramCurveBox = new Gtk::VBox ();
	paramCurveBox->set_spacing(0);

	paramCurve = Gtk::manage (new MyDiagonalCurve ());
	paramCurve->set_size_request (GRAPH_SIZE+2*RADIUS, GRAPH_SIZE+2*RADIUS);
	paramCurve->setType (DCT_Parametric);

	shcSelector = Gtk::manage (new SHCSelector ());
	shcSelector->set_size_request (GRAPH_SIZE-100, 12); // width, height
	//* shcSelector->set_size_request ((GRAPH_SIZE+2*RADIUS)-20, 20);

	paramCurveBox->pack_start (*paramCurve, Gtk::PACK_EXPAND_WIDGET, 0);
	paramCurveBox->pack_start (*shcSelector, Gtk::PACK_EXPAND_WIDGET, 0);

	Gtk::HBox* parambbox = Gtk::manage (new Gtk::HBox ());
	parambbox->set_spacing(4);
	pasteParam = Gtk::manage (new Gtk::Button ());
	pasteParam->add (*Gtk::manage (new RTImage ("edit-paste.png")));
	copyParam = Gtk::manage (new Gtk::Button ());
	copyParam->add (*Gtk::manage (new RTImage ("edit-copy.png")));
	saveParam = Gtk::manage (new Gtk::Button ());
	saveParam->add (*Gtk::manage (new RTImage ("gtk-save-large.png")));
	loadParam = Gtk::manage (new Gtk::Button ());
	loadParam->add (*Gtk::manage (new RTImage ("gtk-open.png")));
	editParam = Gtk::manage (new Gtk::ToggleButton());
	editParam->add (*Gtk::manage (new RTImage ("editmodehand.png")));
	editParam->hide();
	parambbox->pack_end (*pasteParam, Gtk::PACK_SHRINK, 0);
	parambbox->pack_end (*copyParam, Gtk::PACK_SHRINK, 0);
	parambbox->pack_end (*saveParam, Gtk::PACK_SHRINK, 0);
	parambbox->pack_end (*loadParam, Gtk::PACK_SHRINK, 0);
	parambbox->pack_start(*editParam, Gtk::PACK_SHRINK, 0);

	saveParam->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::savePressed) );
	loadParam->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::loadPressed) );
	pasteParam->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::pastePressed) );
	copyParam->signal_clicked().connect( sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::copyPressed) );
	editCustomConn = editParam->signal_toggled().connect( sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::editToggled), editParam) );

	saveParam->set_tooltip_text (M("CURVEEDITOR_TOOLTIPSAVE"));
	loadParam->set_tooltip_text (M("CURVEEDITOR_TOOLTIPLOAD"));
	pasteParam->set_tooltip_text (M("CURVEEDITOR_TOOLTIPPASTE"));
	copyParam->set_tooltip_text (M("CURVEEDITOR_TOOLTIPCOPY"));

	paramCurveBox->pack_end (*parambbox, Gtk::PACK_EXPAND_WIDGET, 0);

	highlights = Gtk::manage (new Adjuster (M("CURVEEDITOR_HIGHLIGHTS"), -100, 100, 1, 0));
	lights     = Gtk::manage (new Adjuster (M("CURVEEDITOR_LIGHTS"), -100, 100, 1, 0));
	darks      = Gtk::manage (new Adjuster (M("CURVEEDITOR_DARKS"), -100, 100, 1, 0));
	shadows    = Gtk::manage (new Adjuster (M("CURVEEDITOR_SHADOWS"), -100, 100, 1, 0));

	Gtk::EventBox* evhighlights = Gtk::manage (new Gtk::EventBox ());
	Gtk::EventBox* evlights = Gtk::manage (new Gtk::EventBox ());
	Gtk::EventBox* evdarks = Gtk::manage (new Gtk::EventBox ());
	Gtk::EventBox* evshadows = Gtk::manage (new Gtk::EventBox ());

	evhighlights->add (*highlights);
	evlights->add (*lights);
	evdarks->add (*darks);
	evshadows->add (*shadows);

	paramCurveBox->pack_start (*evhighlights);
	paramCurveBox->pack_start (*evlights);
	paramCurveBox->pack_start (*evdarks);
	paramCurveBox->pack_start (*evshadows);

	paramCurveBox->show_all ();
	// parametric curve end



	customCurveBox->reference ();
	paramCurveBox->reference ();

	customCurve->setCurveListener (parent); // Send the message directly to the parent
	NURBSCurve->setCurveListener (parent);
	paramCurve->setCurveListener (parent);
	shcSelector->setSHCListener (this);

	highlights->setAdjusterListener (this);
	lights->setAdjusterListener (this);
	darks->setAdjusterListener (this);
	shadows->setAdjusterListener (this);

	evhighlights->set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
	evlights->set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
	evdarks->set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
	evshadows->set_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK);
	evhighlights->signal_enter_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterEntered), 4));
	evlights->signal_enter_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterEntered), 5));
	evdarks->signal_enter_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterEntered), 6));
	evshadows->signal_enter_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterEntered), 7));
	evhighlights->signal_leave_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterLeft), 4));
	evlights->signal_leave_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterLeft), 5));
	evdarks->signal_leave_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterLeft), 6));
	evshadows->signal_leave_notify_event().connect (sigc::bind(sigc::mem_fun(*this, &DiagonalCurveEditorSubGroup::adjusterLeft), 7));
}

DiagonalCurveEditorSubGroup::~DiagonalCurveEditorSubGroup() {
	delete customCurveBox;
	delete paramCurveBox;
	delete NURBSCurveBox;
}

/*
 * Add a new curve to the curves list
 */
DiagonalCurveEditor* DiagonalCurveEditorSubGroup::addCurve(Glib::ustring curveLabel) {
	DiagonalCurveEditor* newCE = new DiagonalCurveEditor(curveLabel, parent, this);

	// Initialization of the new curve
	storeCurveValues(newCE, getCurveFromGUI(DCT_Spline));
	storeCurveValues(newCE, getCurveFromGUI(DCT_Parametric));
	storeCurveValues(newCE, getCurveFromGUI(DCT_NURBS));
	return newCE;
}

/*
 * Switch off the edit button
 */
void DiagonalCurveEditorSubGroup::editModeSwitchedOff () {
	// toggling off all edit buttons, even if only one is toggle on
	bool prevState;
	prevState = editCustomConn.block(true);
	editCustom->set_active(false);
	customCurve->pipetteMouseOver(NULL, 0);
	customCurve->setDirty(true);
	if (!prevState) editCustomConn.block(false);
	prevState = editNURBSConn.block(true);
	editNURBS->set_active(false);
	NURBSCurve->pipetteMouseOver(NULL, 0);
	NURBSCurve->setDirty(true);
	if (!prevState) editNURBSConn.block(false);
	prevState = editParamConn.block(true);
	editParam->set_active(false);
	paramCurve->pipetteMouseOver(NULL, 0);
	paramCurve->setDirty(true);
	if (!prevState) editParamConn.block(false);
}

void DiagonalCurveEditorSubGroup::pipetteMouseOver(EditDataProvider *provider, int modifierKey) {
	CurveEditor *curveEditor = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
	switch((DiagonalCurveType)(curveEditor->curveType->getSelected())) {
	case (DCT_Spline):
		customCurve->pipetteMouseOver(provider, modifierKey);
		customCurve->setDirty(true);
		break;
	case (DCT_Parametric):
		paramCurve->pipetteMouseOver(provider, modifierKey);
		paramCurve->setDirty(true);
		break;
	case (DCT_NURBS):
		NURBSCurve->pipetteMouseOver(provider, modifierKey);
		NURBSCurve->setDirty(true);
		break;
	default:	// (DCT_Linear, DCT_Unchanged)
		// ... do nothing
		break;
	}
}

void DiagonalCurveEditorSubGroup::pipetteButton1Pressed(EditDataProvider *provider, int modifierKey) {
	CurveEditor *curveEditor = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
	switch((DiagonalCurveType)(curveEditor->curveType->getSelected())) {
	case (DCT_Spline):
		customCurve->pipetteButton1Pressed(provider, modifierKey);
		break;
	case (DCT_Parametric):
		paramCurve->pipetteButton1Pressed(provider, modifierKey);
		break;
	case (DCT_NURBS):
		NURBSCurve->pipetteButton1Pressed(provider, modifierKey);
		break;
	default:	// (DCT_Linear, DCT_Unchanged)
		// ... do nothing
		break;
	}
}

void DiagonalCurveEditorSubGroup::pipetteButton1Released(EditDataProvider *provider) {
	CurveEditor *curveEditor = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
	switch((DiagonalCurveType)(curveEditor->curveType->getSelected())) {
	case (DCT_Spline):
		customCurve->pipetteButton1Released(provider);
		break;
	case (DCT_Parametric):
		paramCurve->pipetteButton1Released(provider);
		break;
	case (DCT_NURBS):
		NURBSCurve->pipetteButton1Released(provider);
		break;
	default:	// (DCT_Linear, DCT_Unchanged)
		// ... do nothing
		break;
	}
}

void DiagonalCurveEditorSubGroup::pipetteDrag(EditDataProvider *provider, int modifierKey) {
	CurveEditor *curveEditor = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
	switch((DiagonalCurveType)(curveEditor->curveType->getSelected())) {
	case (DCT_Spline):
		customCurve->pipetteDrag(provider, modifierKey);
		break;
	case (DCT_Parametric):
		paramCurve->pipetteDrag(provider, modifierKey);
		break;
	case (DCT_NURBS):
		NURBSCurve->pipetteDrag(provider, modifierKey);
		break;
	default:	// (DCT_Linear, DCT_Unchanged)
		// ... do nothing
		break;
	}
}

/*
 * Force the resize of the curve editor, if the displayed one is the requested one
 */
void DiagonalCurveEditorSubGroup::refresh(CurveEditor *curveToRefresh) {
	if (curveToRefresh != NULL && curveToRefresh == static_cast<DiagonalCurveEditor*>(parent->displayedCurve)) {
		switch((DiagonalCurveType)(curveToRefresh->curveType->getSelected())) {
		case (DCT_Spline):
			customCurve->refresh();
			break;
		case (DCT_Parametric):
			paramCurve->refresh();
			shcSelector->refresh();
			break;
		case (DCT_NURBS):
			NURBSCurve->refresh();
			break;
		default:	// (DCT_Linear, DCT_Unchanged)
			// ... do nothing
			break;
		}
	}
}

/*
 * Switch the editor widgets to the currently edited curve
 */
void DiagonalCurveEditorSubGroup::switchGUI() {

	removeEditor();

	DiagonalCurveEditor* dCurve = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);

	if (dCurve) {

		// Initializing GUI values + repacking the appropriated widget
		//dCurve->typeconn.block(true);

		// first we update the colored bar

		ColorProvider *barColorProvider = dCurve->getLeftBarColorProvider();
		std::vector<GradientMilestone>  bgGradient = dCurve->getLeftBarBgGradient();
		if (barColorProvider == NULL && bgGradient.size() == 0) {
			// dCurve has no left colored bar, so we delete the object
			if (leftBar) {
				delete leftBar;
				leftBar = NULL;
			}
		}
		else {
			// dCurve ave a ColorProvider or a background gradient defined, so we create/update the object
			if (!leftBar) {
				leftBar = new ColoredBar(RTO_Bottom2Top);
			}
			if (barColorProvider) {
				bgGradient.clear();
				leftBar->setColorProvider(barColorProvider, dCurve->getLeftBarCallerId());
				leftBar->setBgGradient (bgGradient);
			}
			else {
				leftBar->setColorProvider(NULL, -1);
				leftBar->setBgGradient (bgGradient);
			}
		}

		barColorProvider = dCurve->getBottomBarColorProvider();
		bgGradient = dCurve->getBottomBarBgGradient();
		if (barColorProvider == NULL && bgGradient.size() == 0) {
			// dCurve has no bottom colored bar, so we delete the object
			if (bottomBar) {
				delete bottomBar;
				bottomBar = NULL;
			}
		}
		else {
			// dCurve has a ColorProvider or a background gradient defined, so we create/update the object
			if (!bottomBar) {
				bottomBar = new ColoredBar(RTO_Left2Right);
			}
			if (barColorProvider) {
				bgGradient.clear();
				bottomBar->setColorProvider(barColorProvider, dCurve->getBottomBarCallerId());
				bottomBar->setBgGradient (bgGradient);
			}
			else {
				bottomBar->setColorProvider(NULL, -1);
				bottomBar->setBgGradient (bgGradient);
			}
		}

		switch((DiagonalCurveType)(dCurve->curveType->getSelected())) {
		case (DCT_Spline):
			customCurve->setPoints (dCurve->customCurveEd);
			customCurve->setColorProvider(dCurve->getCurveColorProvider(), dCurve->getCurveCallerId());
			customCurve->setColoredBar(leftBar, bottomBar);
			customCurve->forceResize();
			updateEditButton(dCurve, editCustom, editCustomConn);
			parent->pack_start (*customCurveBox);
			customCurveBox->check_resize();
			break;
		case (DCT_Parametric):
		{
			Glib::ustring label[4];
			dCurve->getRangeLabels(label[0], label[1], label[2], label[3]);
			double mileStone[3];
			dCurve->getRangeDefaultMilestones(mileStone[0], mileStone[1], mileStone[2]);
			paramCurve->setPoints (dCurve->paramCurveEd);
			shcSelector->setDefaults(mileStone[0], mileStone[1], mileStone[2]);
			shcSelector->setPositions (
					dCurve->paramCurveEd.at(1),
					dCurve->paramCurveEd.at(2),
					dCurve->paramCurveEd.at(3)
			);

			highlights->setValue (dCurve->paramCurveEd.at(4));
			highlights->setLabel(label[3]);
			lights->setValue (dCurve->paramCurveEd.at(5));
			lights->setLabel(label[2]);
			darks->setValue (dCurve->paramCurveEd.at(6));
			darks->setLabel(label[1]);
			shadows->setValue (dCurve->paramCurveEd.at(7));
			shadows->setLabel(label[0]);
			shcSelector->setColorProvider(barColorProvider, dCurve->getBottomBarCallerId());
			shcSelector->setBgGradient(bgGradient);
			shcSelector->setMargins( (leftBar ? MyCurve::getBarWidth()+CBAR_MARGIN : RADIUS), RADIUS );
			paramCurve->setColoredBar(leftBar, NULL);
			paramCurve->forceResize();
			updateEditButton(dCurve, editParam, editParamConn);
			parent->pack_start (*paramCurveBox);
			break;
		}
		case (DCT_NURBS):
			NURBSCurve->setPoints (dCurve->NURBSCurveEd);
			NURBSCurve->setColorProvider(dCurve->getCurveColorProvider(), dCurve->getCurveCallerId());
			NURBSCurve->setColoredBar(leftBar, bottomBar);
			NURBSCurve->forceResize();
			updateEditButton(dCurve, editNURBS, editNURBSConn);
			parent->pack_start (*NURBSCurveBox);
			NURBSCurveBox->check_resize();
			break;
		default:	// (DCT_Linear, DCT_Unchanged)
			// ... do nothing
			break;
		}

		//dCurve->typeconn.block(false);
	}
}

void DiagonalCurveEditorSubGroup::savePressed () {

	Glib::ustring fname = outputFile();
	if (fname.size()) {
		std::ofstream f (fname.c_str());
		std::vector<double> p;
		//std::vector<double> p = customCurve->getPoints ();

		switch (parent->displayedCurve->selected) {
		case DCT_Spline:	// custom
			p = customCurve->getPoints ();
			break;
		case DCT_NURBS:		// NURBS
			p = NURBSCurve->getPoints ();
			break;
		case DCT_Parametric:
			p = paramCurve->getPoints ();
			break;
		default:
			break;
		}

		int ix = 0;
		if (p[ix]==(double)(DCT_Linear))
			f << "Linear" << std::endl;
		else if (p[ix]==(double)(DCT_Spline))
			f << "Spline" << std::endl;
		else if (p[ix]==(double)(DCT_NURBS))
			f << "NURBS" << std::endl;
		else if (p[ix]==(double)(DCT_Parametric))
			f << "Parametric" << std::endl;
		if (p[ix]==(double)(DCT_Parametric)) {
			ix++;
			for (unsigned int i=0; i<p.size()-1; i++, ix++)
			f << p[ix] << std::endl;
		}
		else {	
			ix++;
			for (unsigned int i=0; i<p.size()/2; i++, ix+=2)
				f << p[ix] << ' ' << p[ix+1] << std::endl;
		}

		f.close ();
	}
}

void DiagonalCurveEditorSubGroup::loadPressed () {

	Glib::ustring fname = inputFile();
	if (fname.size()) {
		std::ifstream f (fname.c_str());
		if (f) {
			std::vector<double> p;
			std::string s;
			f >> s;
			if (s=="Linear")
				p.push_back ((double)(DCT_Linear));
			else if (s=="Spline")
				p.push_back ((double)(DCT_Spline));
			else if (s=="NURBS")
				p.push_back ((double)(DCT_NURBS));
			else if (s=="Parametric")
				p.push_back ((double)(DCT_Parametric));
			else return;

			double x;
			while (f) {
				f >> x;
				if (f)
					p.push_back (x);
			}
			if (p[0] == (double)(DCT_Spline)) {
				customCurve->setPoints (p);
				customCurve->queue_draw ();
				customCurve->notifyListener ();
			}
			else if (p[0] == (double)(DCT_NURBS)) {
				NURBSCurve->setPoints (p);
				NURBSCurve->queue_draw ();
				NURBSCurve->notifyListener ();
			}
			else if (p[0] == (double)(DCT_Parametric)) {
				shcSelector->setPositions ( p[1], p[2], p[3] );
				highlights->setValue (p[4]);
				lights->setValue (p[5]);
				darks->setValue (p[6]);
				shadows->setValue (p[7]);
				paramCurve->setPoints (p);
				paramCurve->queue_draw ();
				paramCurve->notifyListener ();
			}
		}
	}
}

void DiagonalCurveEditorSubGroup::copyPressed () {
// For compatibility use enum DiagonalCurveType here

	std::vector<double> curve;

	switch (parent->displayedCurve->selected) {
	case DCT_Spline:                // custom
		curve = customCurve->getPoints ();
		clipboard.setDiagonalCurveData (curve,DCT_Spline);
		break;
	case DCT_Parametric:            // parametric
		// ... do something, first add save/load functions
		curve = paramCurve->getPoints ();
		clipboard.setDiagonalCurveData (curve,DCT_Parametric);
		break;
	case DCT_NURBS:                 // NURBS
		curve = NURBSCurve->getPoints ();
		clipboard.setDiagonalCurveData (curve,DCT_NURBS);
		break;
	default:                       // (DCT_Linear, DCT_Unchanged)
		// ... do nothing
		break;
	}
}

void DiagonalCurveEditorSubGroup::pastePressed () {
// For compatibility use enum DiagonalCurveType here

	std::vector<double> curve;
	DiagonalCurveType type;

	type = clipboard.hasDiagonalCurveData();

	if (type == (DiagonalCurveType)parent->displayedCurve->selected) {
		curve = clipboard.getDiagonalCurveData ();
		switch (type) {
		case DCT_Spline:           // custom
			customCurve->setPoints (curve);
			customCurve->queue_draw ();
			customCurve->notifyListener ();
			break;
		case DCT_Parametric:       // parametric
			// ... do something, first add save/load functions
			shcSelector->setPositions (
			curve[1],
			curve[2],
			curve[3]  );
			highlights->setValue (curve[4]);
			lights->setValue (curve[5]);
			darks->setValue (curve[6]);
			shadows->setValue (curve[7]);
			paramCurve->setPoints (curve);
			paramCurve->queue_draw ();
			paramCurve->notifyListener ();
			break;
		case DCT_NURBS:            // NURBS
			NURBSCurve->setPoints (curve);
			NURBSCurve->queue_draw ();
			NURBSCurve->notifyListener ();
			break;
		default:                   // (DCT_Linear, DCT_Unchanged)
			// ... do nothing
			break;
		}
	}
	return;
}

void DiagonalCurveEditorSubGroup::editToggled (Gtk::ToggleButton *button) {
	DiagonalCurveEditor* dCurve = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
	if (!dCurve)
		// should never happen!
		return;

	if (button->get_active()) {
		dCurve->subscribe();
		if (button == editCustom)
			customCurve->notifyListener ();
		else if (button == editNURBS)
			NURBSCurve->notifyListener ();
		else if (button == editParam)
			paramCurve->notifyListener ();

		/*
		if (button != editCustom) {
			editCustomConn.block(true);
			editCustom->set_active(true);
			editCustomConn.block(false);
		}
		else {
			// will throw the event of curveChanged, which will now build the edit's buffer
			customCurve->notifyListener ();
		}
		if (button != editNURBS) {
			editNURBSConn.block(true);
			editNURBS->set_active(true);
			editNURBSConn.block(false);
		}
		else {
			NURBSCurve->notifyListener ();
		}
		if (button != editParam) {
			editParamConn.block(true);
			editParam->set_active(true);
			editParamConn.block(false);
		}
		else {
			paramCurve->notifyListener ();
		}
		*/
	}
	else {
		dCurve->unsubscribe();
		/*
		if (button != editCustom ) { editCustomConn.block(true);  editCustom->set_active(false);  editCustomConn.block(false); }
		if (button != editNURBS  ) { editNURBSConn.block(true);   editNURBS->set_active(false);   editNURBSConn.block(false);  }
		if (button != editParam  ) { editParamConn.block(true);   editParam->set_active(false);   editParamConn.block(false);  }
		*/
	}
}

/*
 * Store the curves of the currently displayed type from the widgets to the CurveEditor object
 */
void DiagonalCurveEditorSubGroup::storeDisplayedCurve() {
	if (parent->displayedCurve) {
		switch (parent->displayedCurve->selected) {
		case (DCT_Spline):
			storeCurveValues(parent->displayedCurve, getCurveFromGUI(DCT_Spline));
			break;
		case (DCT_Parametric):
			storeCurveValues(parent->displayedCurve, getCurveFromGUI(DCT_Parametric));
			break;
		case (DCT_NURBS):
			storeCurveValues(parent->displayedCurve, getCurveFromGUI(DCT_NURBS));
			break;
		default:
			break;
		}
	}
}

/*
 * Restore the histogram to all types from the CurveEditor object to the widgets
 */
void DiagonalCurveEditorSubGroup::restoreDisplayedHistogram() {
	if (parent->displayedCurve /*&& initslope==1*/) {
		paramCurve->updateBackgroundHistogram (parent->displayedCurve->histogram);
		customCurve->updateBackgroundHistogram (parent->displayedCurve->histogram);
		NURBSCurve->updateBackgroundHistogram (parent->displayedCurve->histogram);
	}
	
}

void DiagonalCurveEditorSubGroup::storeCurveValues (CurveEditor* ce, const std::vector<double>& p) {
	if (!p.empty()) {
		DiagonalCurveType t = (DiagonalCurveType)p[0];

		switch (t) {
		case (DCT_Spline):
			(static_cast<DiagonalCurveEditor*>(ce))->customCurveEd = p;
			break;
		case (DCT_Parametric):
			(static_cast<DiagonalCurveEditor*>(ce))->paramCurveEd = p;
			break;
		case (DCT_NURBS):
			(static_cast<DiagonalCurveEditor*>(ce))->NURBSCurveEd = p;
			break;
		default:
			break;
		}
	}
}

/*
 * Called to update the parametric curve graph with new slider values
 */
const std::vector<double> DiagonalCurveEditorSubGroup::getCurveFromGUI (int type) {
	switch ((DiagonalCurveType)type) {
	case (DCT_Parametric): {
		std::vector<double> lcurve (8);
		lcurve[0] = (double)(DCT_Parametric);
		shcSelector->getPositions (lcurve[1], lcurve[2], lcurve[3]);
		lcurve[4] = highlights->getValue ();
		lcurve[5] = lights->getValue ();
		lcurve[6] = darks->getValue ();
		lcurve[7] = shadows->getValue ();
		return lcurve;
		}
	case (DCT_Spline):
		return customCurve->getPoints ();
	case (DCT_NURBS):
		return NURBSCurve->getPoints ();
	default: {
		// linear and other solutions
		std::vector<double> lcurve (1);
		lcurve[0] = (double)(DCT_Linear);
		return lcurve;
		}
	}
}

/*
 * Unlink the tree editor widgets from their parent box to hide them
 */
void DiagonalCurveEditorSubGroup::removeEditor () {
	removeIfThere (parent, customCurveBox, false);
	removeIfThere (parent, paramCurveBox, false);
	removeIfThere (parent, NURBSCurveBox, false);
}

bool DiagonalCurveEditorSubGroup::curveReset(int cType, double iValue) {
	switch ((DiagonalCurveType) cType) {
	case (DCT_NURBS) :	// = Control cage
		NURBSCurve->reset ();
		return true;
		break;
	case (DCT_Spline) :	// = Custom
		customCurve->reset ();
		return true;
		break;
	case (DCT_Parametric) :
	{
		DiagonalCurveEditor* dCurve = static_cast<DiagonalCurveEditor*>(parent->displayedCurve);
		double mileStone[3];
		dCurve->getRangeDefaultMilestones(mileStone[0], mileStone[1], mileStone[2]);

		highlights->resetPressed(NULL);
		lights->resetPressed(NULL);
		darks->resetPressed(NULL);
		shadows->resetPressed(NULL);
		shcSelector->setDefaults(mileStone[0], mileStone[1], mileStone[2]);
		shcSelector->reset();
		paramCurve->reset ();
		return true;
		break;
	}
	default:
		return false;
		break;
	}
	return true;
}

/*
 * Listener
 */
void DiagonalCurveEditorSubGroup::shcChanged () {

    paramCurve->setPoints (getCurveFromGUI(DCT_Parametric));
	storeDisplayedCurve();
	parent->curveChanged ();
}

/*
 * Listener
 */
void DiagonalCurveEditorSubGroup::adjusterChanged (Adjuster* a, double newval) {

	paramCurve->setPoints (getCurveFromGUI(DCT_Parametric));
	storeDisplayedCurve();
	parent->curveChanged ();
}

/*
 * Listener called when the mouse is over a parametric curve's slider
 */
bool DiagonalCurveEditorSubGroup::adjusterEntered (GdkEventCrossing* ev, int ac) {

	if (ev->detail != GDK_NOTIFY_INFERIOR) {
		activeParamControl = ac;
		paramCurve->setActiveParam (activeParamControl);
	}
	return true;
}

/*
 * Listener called when the mouse left the parametric curve's slider
 */
bool DiagonalCurveEditorSubGroup::adjusterLeft (GdkEventCrossing* ev, int ac) {

	if (ev->detail != GDK_NOTIFY_INFERIOR) {
		activeParamControl = -1;
		paramCurve->setActiveParam (activeParamControl);
	}
	return true;
}

void DiagonalCurveEditorSubGroup::updateBackgroundHistogram (CurveEditor* ce) {
	if (ce==parent->displayedCurve /*&&  initslope==1*/) {
		paramCurve->updateBackgroundHistogram (ce->histogram);
		customCurve->updateBackgroundHistogram (ce->histogram);
		NURBSCurve->updateBackgroundHistogram (ce->histogram);
	}
}

void DiagonalCurveEditorSubGroup::setSubGroupRangeLabels(Glib::ustring r1, Glib::ustring r2, Glib::ustring r3, Glib::ustring r4) {
	shadows->setLabel(r1);
	darks->setLabel(r2);
	lights->setLabel(r3);
	highlights->setLabel(r4);
}
