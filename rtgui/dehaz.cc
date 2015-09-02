/*
 *  This file is part of RawTherapee.
 */
#include "dehaz.h"
#include "mycurve.h"

using namespace rtengine;
using namespace rtengine::procparams;

Dehaz::Dehaz () : FoldableToolPanel(this, "dehaz", M("TP_DEHAZ_LABEL"), false, true)
{
    CurveListener::setMulti(true);
    std::vector<GradientMilestone> milestones;
    
    dehazFrame = Gtk::manage (new Gtk::Frame (M("TP_DEHAZ_LAB")) );
    dehazFrame->set_tooltip_text(M("TP_DEHAZ_LAB_TOOLTIP"));
    dehazFrame->set_border_width(0);
    dehazFrame->set_label_align(0.025, 0.5);
    
    Gtk::VBox * dehazVBox = Gtk::manage ( new Gtk::VBox());
    dehazVBox->set_border_width(4);
    dehazVBox->set_spacing(2);

    Gtk::VBox * RetiVBox = Gtk::manage ( new Gtk::VBox());
    RetiVBox->set_border_width(4);
    RetiVBox->set_spacing(2);
    
    dhbox = Gtk::manage (new Gtk::HBox ());
    labmdh = Gtk::manage (new Gtk::Label (M("TP_DEHAZE_MET") + ":"));
    dhbox->pack_start (*labmdh, Gtk::PACK_SHRINK, 1);

    dehazmet = Gtk::manage (new MyComboBoxText ());
    dehazmet->append_text (M("TP_DEHAZ_LOW"));
    dehazmet->append_text (M("TP_DEHAZ_UNI"));
    dehazmet->append_text (M("TP_DEHAZ_HIGH"));
    dehazmet->set_active(0);
    dehazmetConn = dehazmet->signal_changed().connect ( sigc::mem_fun(*this, &Dehaz::dehazmetChanged) );
    dehazmet->set_tooltip_markup (M("TP_DEHAZ_MET_TOOLTIP"));
    dhbox->pack_start(*dehazmet);
    dehazVBox->pack_start(*dhbox);
    std::vector<double> defaultCurve;
   
    curveEditorGD = new CurveEditorGroup (options.lastDehazDir, M("TP_DEHAZ_CONTEDIT"));
    curveEditorGD->setCurveListener (this);
    rtengine::DehazParams::getDefaultCDCurve(defaultCurve);
    cdshape = static_cast<DiagonalCurveEditor*>(curveEditorGD->addCurve(CT_Diagonal, M("TP_DEHAZ_CURVEEDITOR_CD")));
    cdshape->setResetCurve(DiagonalCurveType(defaultCurve.at(0)), defaultCurve);
    cdshape->setTooltip(M("TP_DEHAZ_CURVEEDITOR_CD_TOOLTIP"));
    std::vector<GradientMilestone> milestones22;

    milestones22.push_back( GradientMilestone(0., 0., 0., 0.) );
    milestones22.push_back( GradientMilestone(1., 1., 1., 1.) );
    cdshape->setBottomBarBgGradient(milestones22);
    cdshape->setLeftBarBgGradient(milestones22);

    curveEditorGD->curveListComplete();

    transmissionCurveEditorG = new CurveEditorGroup (options.lastDehazDir, M("TP_DEHAZ_TRANSMISSION"));
    transmissionCurveEditorG->setCurveListener (this);

    rtengine::DehazParams::getDefaulttransmissionCurve(defaultCurve);
    transmissionShape = static_cast<FlatCurveEditor*>(transmissionCurveEditorG->addCurve(CT_Flat, "", NULL, false));
    transmissionShape->setIdentityValue(0.);
    transmissionShape->setResetCurve(FlatCurveType(defaultCurve.at(0)), defaultCurve);
    transmissionShape->setBottomBarBgGradient(milestones);
    transmissionCurveEditorG->set_tooltip_markup (M("TP_DEHAZ_TRANS_TOOLTIP"));

    transmissionCurveEditorG->curveListComplete();

   
    
    str = Gtk::manage (new Adjuster (M("TP_DEHAZ_STR"), 0, 100., 1., 60.));
    str->set_tooltip_markup (M("TP_DEHAZ_STR_TOOLTIP"));
    neigh = Gtk::manage (new Adjuster (M("TP_DEHAZ_NEIGH"), 6, 100., 1., 80.));
    
    retinex = Gtk::manage (new Gtk::CheckButton (M("TP_DEHAZ_RETIN")));
    retinex->set_active (true);
    retinexConn  = retinex->signal_toggled().connect( sigc::mem_fun(*this, &Dehaz::retinexChanged) );
    
    dehazVBox->pack_start (*str);
    str->show ();
    
    dehazVBox->pack_start (*neigh);
    neigh->show ();
    neigh->set_tooltip_markup (M("TP_DEHAZ_NEIGH_TOOLTIP"));
    
    dehazVBox->pack_start (*retinex);
    retinex->show ();
    
    scal   = Gtk::manage (new Adjuster (M("TP_DEHAZ_SCAL"), 1, 6., 1., 3.));
    gain   = Gtk::manage (new Adjuster (M("TP_DEHAZ_GAIN"), 50, 150, 1, 100));
    offs   = Gtk::manage (new Adjuster (M("TP_DEHAZ_OFFS"), -3000, 3000, 1, 0));
    vart   = Gtk::manage (new Adjuster (M("TP_DEHAZ_VART"), 80, 250, 1, 125));
    gain->set_tooltip_markup (M("TP_DEHAZ_GAIN_TOOLTIP"));
    scal->set_tooltip_markup (M("TP_DEHAZ_SCAL_TOOLTIP"));
    vart->set_tooltip_markup (M("TP_DEHAZ_VART_TOOLTIP"));
   
    RetiVBox->pack_start (*curveEditorGD, Gtk::PACK_SHRINK, 4);
    curveEditorGD->show();
    
    RetiVBox->pack_start (*scal);
    scal->show ();


    RetiVBox->pack_start (*gain);
    gain->show ();

    RetiVBox->pack_start (*offs);
    offs->show ();
    
    RetiVBox->pack_start (*vart);
    vart->show ();

    RetiVBox->pack_start( *transmissionCurveEditorG, Gtk::PACK_SHRINK, 2);
    transmissionCurveEditorG->show();
   
    
    str->setAdjusterListener (this);
    scal->setAdjusterListener (this);
    neigh->setAdjusterListener (this);
    gain->setAdjusterListener (this);
    offs->setAdjusterListener (this);
    vart->setAdjusterListener (this);
    pack_start (*dehazVBox);
    dehazFrame->add(*RetiVBox);
    pack_start (*dehazFrame);
    dehazFrame->hide();
    
    disableListener();
    retinexChanged();
    enableListener();

}

Dehaz::~Dehaz()
{
    delete curveEditorGD;
    delete transmissionCurveEditorG;
    
}


void Dehaz::read (const ProcParams* pp, const ParamsEdited* pedited)
{
    disableListener ();
    dehazmetConn.block(true);
    

    if (pedited) {
        scal->setEditedState (pedited->dehaz.scal ? Edited : UnEdited);
        neigh->setEditedState (pedited->dehaz.neigh ? Edited : UnEdited);
        gain->setEditedState (pedited->dehaz.gain ? Edited : UnEdited);
        offs->setEditedState (pedited->dehaz.offs ? Edited : UnEdited);
        vart->setEditedState (pedited->dehaz.vart ? Edited : UnEdited);
        set_inconsistent (multiImage && !pedited->dehaz.enabled);
        retinex->set_inconsistent (!pedited->dehaz.retinex);

    
    if (!pedited->dehaz.dehazmet) {
            dehazmet->set_active_text(M("GENERAL_UNCHANGED"));
    }
    cdshape->setUnChanged  (!pedited->dehaz.cdcurve);
    transmissionShape->setUnChanged (!pedited->dehaz.transmissionCurve);
    
    }
    
    neigh->setValue    (pp->dehaz.neigh);
    gain->setValue      (pp->dehaz.gain);
    offs->setValue  (pp->dehaz.offs);
    str->setValue    (pp->dehaz.str);
    scal->setValue      (pp->dehaz.scal);
    vart->setValue  (pp->dehaz.vart);

    setEnabled (pp->dehaz.enabled);

    retinexConn.block (true);
    retinex->set_active (pp->dehaz.retinex);
    retinexConn.block (false);
    lastretinex = pp->dehaz.retinex;
    
    
    if (pp->dehaz.dehazmet == "low") {
        dehazmet->set_active (0);
    } else if (pp->dehaz.dehazmet == "uni") {
        dehazmet->set_active (1);
    } else if (pp->dehaz.dehazmet == "high") {
        dehazmet->set_active (2);
    }    
    dehazmetChanged ();
    retinexConn.block(false);    
    retinexChanged ();
    retinexConn.block(false);
    
    cdshape->setCurve  (pp->dehaz.cdcurve);
    dehazmetConn.block(false);
    transmissionShape->setCurve (pp->dehaz.transmissionCurve);


    enableListener ();
}
void Dehaz::retinexUpdateUI ()
{
    if (!batchMode) {
        if (retinex->get_active ()) {
            scal->show();
            gain->show();
            offs->show();
            vart->show();
            transmissionCurveEditorG->show();
            curveEditorGD->show();
            dehazFrame->show();
        } else  {
            scal->hide();
            gain->hide();
            offs->hide();
            vart->hide();
            transmissionCurveEditorG->hide();
            curveEditorGD->hide();
            dehazFrame->hide();
        }
    }
}



void Dehaz::write (ProcParams* pp, ParamsEdited* pedited)
{
    
    pp->dehaz.str    = str->getValue ();
    pp->dehaz.scal      = (int)scal->getValue ();
    pp->dehaz.neigh    = neigh->getValue ();
    pp->dehaz.gain      = (int)gain->getValue ();
    pp->dehaz.offs  = (int)offs->getValue ();
    pp->dehaz.vart  = (int)vart->getValue ();
    pp->dehaz.cdcurve = cdshape->getCurve ();
    pp->dehaz.transmissionCurve = transmissionShape->getCurve ();
    pp->dehaz.enabled      = getEnabled();
    pp->dehaz.retinex                = retinex->get_active();
    
    if (pedited) {
        pedited->dehaz.dehazmet    = dehazmet->get_active_text() != M("GENERAL_UNCHANGED");
       
        //%%%%%%%%%%%%%%%%%%%%%%
        pedited->dehaz.str   = str->getEditedState ();
        pedited->dehaz.scal     = scal->getEditedState ();
        pedited->dehaz.neigh   = neigh->getEditedState ();
        pedited->dehaz.gain     = gain->getEditedState ();
        pedited->dehaz.offs = offs->getEditedState ();
        pedited->dehaz.vart = vart->getEditedState ();
        pedited->dehaz.cdcurve   = !cdshape->isUnChanged ();
        pedited->dehaz.transmissionCurve  = !transmissionShape->isUnChanged ();
        pedited->dehaz.enabled       = !get_inconsistent();
        pedited->dehaz.retinex       = !retinex->get_inconsistent();

    }
    if (dehazmet->get_active_row_number() == 0) {
        pp->dehaz.dehazmet = "low";
    } else if (dehazmet->get_active_row_number() == 1) {
        pp->dehaz.dehazmet = "uni";
    } else if (dehazmet->get_active_row_number() == 2) {
        pp->dehaz.dehazmet = "high";
    }
}

void Dehaz::dehazmetChanged()
{
    if (listener) { 
        listener->panelChanged (Evdehazmet, dehazmet->get_active_text ());
    }
 }

void Dehaz::retinexChanged ()
{
    if (batchMode) {
        if (retinex->get_inconsistent()) {
            retinex->set_inconsistent (false);
            retinexConn.block (true);
            retinex->set_active (false);
            retinexConn.block (false);
        } else if (lastretinex) {
            retinex->set_inconsistent (true);
        }

        lastretinex = retinex->get_active ();
    }
    retinexUpdateUI();

    if (listener) {
        if (retinex->get_active()) {
            if (getEnabled()) {
                listener->panelChanged (EvDehazretinex, M("GENERAL_ENABLED"));
            }
        } else {
            if (getEnabled()) {
                listener->panelChanged (EvDehazretinex, M("GENERAL_DISABLED"));  
            }
        }

    }
}
 
 
 
void Dehaz::setDefaults (const ProcParams* defParams, const ParamsEdited* pedited)
{

    neigh->setDefault (defParams->dehaz.neigh);
    gain->setDefault (defParams->dehaz.gain);
    offs->setDefault (defParams->dehaz.offs);
    str->setDefault (defParams->dehaz.str);
    scal->setDefault (defParams->dehaz.scal);
    vart->setDefault (defParams->dehaz.vart);

    if (pedited) {
        neigh->setDefaultEditedState (pedited->dehaz.neigh ? Edited : UnEdited);
        gain->setDefaultEditedState (pedited->dehaz.gain ? Edited : UnEdited);
        offs->setDefaultEditedState (pedited->dehaz.offs ? Edited : UnEdited);
        str->setDefaultEditedState (pedited->dehaz.str ? Edited : UnEdited);
        scal->setDefaultEditedState (pedited->dehaz.scal ? Edited : UnEdited);
        vart->setDefaultEditedState (pedited->dehaz.vart ? Edited : UnEdited);

    } else {
        neigh->setDefaultEditedState (Irrelevant);
        gain->setDefaultEditedState (Irrelevant);
        offs->setDefaultEditedState (Irrelevant);
        vart->setDefaultEditedState (Irrelevant);
        str->setDefaultEditedState (Irrelevant);
        scal->setDefaultEditedState (Irrelevant);
    }
}
/*
void Dehaz::setAdjusterBehavior (bool splitAdd, bool satThresholdAdd, bool satOpacityAdd, bool strprotectAdd, bool balanceAdd)
{

}
*/

void Dehaz::adjusterChanged (Adjuster* a, double newval)
{

    if (!listener || !getEnabled()) {
        return;
    }

    if (a == neigh) {
        listener->panelChanged (EvLneigh, neigh->getTextValue());
    } else if (a == str) {
        listener->panelChanged (EvLstr, str->getTextValue());
    } else if (a == scal) {
        listener->panelChanged (EvLscal, scal->getTextValue());
    } else if (a == gain) {
        listener->panelChanged (EvLgain, gain->getTextValue());
    } else if (a == offs) {
        listener->panelChanged (EvLoffs, offs->getTextValue());
    } else if (a == vart) {
        listener->panelChanged (EvLvart, vart->getTextValue());
    }
}



void Dehaz::autoOpenCurve  ()
{
    cdshape->openIfNonlinear();
    transmissionShape->openIfNonlinear();
   
}


void Dehaz::curveChanged (CurveEditor* ce)
{
    if (listener && getEnabled()) {
        if (ce == cdshape) {
            listener->panelChanged (EvLCDCurve, M("HISTORY_CUSTOMCURVE"));
        } else if (ce == transmissionShape) {
            listener->panelChanged (EvDehaztransmission, M("HISTORY_CUSTOMCURVE"));
        }
    }
}

void Dehaz::enabledChanged ()
{

    if (listener) {
        if (get_inconsistent()) {
            listener->panelChanged (EvDehazEnabled, M("GENERAL_UNCHANGED"));
        } else if (getEnabled()) {
            listener->panelChanged (EvDehazEnabled, M("GENERAL_ENABLED"));
        } else {
            listener->panelChanged (EvDehazEnabled, M("GENERAL_DISABLED"));
        }
    }
}


void Dehaz::trimValues (rtengine::procparams::ProcParams* pp)
{
    str->trimValue(pp->dehaz.str);
    scal->trimValue(pp->dehaz.scal);
    neigh->trimValue(pp->dehaz.neigh);
    gain->trimValue(pp->dehaz.gain);
    offs->trimValue(pp->dehaz.offs);
    vart->trimValue(pp->dehaz.vart);

}

void Dehaz::setBatchMode (bool batchMode)
{
    ToolPanel::setBatchMode (batchMode);
    neigh->showEditedCB ();
    gain->showEditedCB ();
    offs->showEditedCB ();
    str->showEditedCB ();
    scal->showEditedCB ();
    vart->showEditedCB ();
    curveEditorGD->setBatchMode (batchMode);
    transmissionCurveEditorG->setBatchMode (batchMode);


}