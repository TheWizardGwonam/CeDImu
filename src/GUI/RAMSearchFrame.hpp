#ifndef RAMSEARCHFRAME_HPP
#define RAMSEARCHFRAME_HPP

class RAMSearchFrame;

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>

#include "MainFrame.hpp"
#include "RAMSearchList.hpp"
#include "../cores/VDSC.hpp"

class RAMSearchFrame : public wxFrame
{
public:
    VDSC* vdsc;
    MainFrame* mainFrame;
    RAMSearchList* ramSearchList;
    wxTimer* renderTimer;

    wxCheckBox* checkMisaligned;
    wxRadioButton* signed_;
    wxRadioButton* unsigned_;
    wxRadioButton* hexadecimal_;
    wxRadioButton* byte1;
    wxRadioButton* byte2;
    wxRadioButton* byte4;
    wxRadioButton* lastByte;

    RAMSearchFrame(VDSC* vds, MainFrame* parent, const wxPoint& pos, const wxSize& size);
    ~RAMSearchFrame();

    void OnClose(wxCloseEvent& event);

    void PaintEvent();
    void RefreshLoop(wxTimerEvent& event);

    void OnCheckMisaligned(wxCommandEvent& event);
    void OnSigned(wxCommandEvent& event);
    void OnUnsigned(wxCommandEvent& event);
    void OnHexadecimal(wxCommandEvent& event);
    void OnByte1(wxCommandEvent& event);
    void OnByte2(wxCommandEvent& event);
    void OnByte4(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif // RAMSEARCHFRAME_HPP
