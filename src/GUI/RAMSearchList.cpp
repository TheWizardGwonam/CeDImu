#include "RAMSearchList.hpp"
#include "../utils.hpp"

#define GET_ARRAY16(array, index) (array[(index)] << 8 | array[(index)+1])
#define GET_ARRAY32(array, index) (array[(index)] << 24 | array[(index)+1] << 16 | array[(index)+2] << 8 | array[(index)+3])

RAMSearchList::RAMSearchList(RAMSearchFrame* parent) : wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_HRULES | wxLC_VRULES)
{
    ramSearchFrame = parent;

    wxListItem addressCol;
    addressCol.SetId(0);
    addressCol.SetText("Address");
    addressCol.SetWidth(100);
    InsertColumn(0, addressCol);

    wxListItem valueCol;
    valueCol.SetId(1);
    valueCol.SetText("Value");
    valueCol.SetWidth(100);
    InsertColumn(1, valueCol);

    EnableAlternateRowColours();
    SetItemCount(ramSearchFrame->mainFrame->app->vdsc->allocatedMemory);
}

wxString RAMSearchList::OnGetItemText(long item, long column) const
{
    if(column == 1)
        if(ramSearchFrame->byte1->GetValue())
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int8_t)ramSearchFrame->vdsc->GetByteNoDebug(item));
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(ramSearchFrame->vdsc->GetByteNoDebug(item));
            else
                return toHex(ramSearchFrame->vdsc->GetByteNoDebug(item));

        else if(ramSearchFrame->byte2->GetValue())
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int16_t)ramSearchFrame->vdsc->GetWordNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*2));
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(ramSearchFrame->vdsc->GetWordNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*2));
            else
                return toHex(ramSearchFrame->vdsc->GetWordNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*2));

        else
            if(ramSearchFrame->signed_->GetValue())
                return std::to_string((int32_t)ramSearchFrame->vdsc->GetLongNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*4));
            else if(ramSearchFrame->unsigned_->GetValue())
                return std::to_string(ramSearchFrame->vdsc->GetLongNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*4));
            else
                return toHex(ramSearchFrame->vdsc->GetLongNoDebug(ramSearchFrame->checkMisaligned->GetValue() ? item : item*4));
    else
        if(ramSearchFrame->byte1->GetValue() || ramSearchFrame->checkMisaligned->GetValue())
            return toHex(item);
        else if(ramSearchFrame->byte2->GetValue())
            return toHex(item*2);
        else
            return toHex(item*4);
}
