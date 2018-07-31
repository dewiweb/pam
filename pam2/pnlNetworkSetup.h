#ifndef PNLNETWORKSETUP_H
#define PNLNETWORKSETUP_H

//(*Headers(pnlNetworkSetup)
#include <wx/notebook.h>
#include "wmbutton.h"
#include "wmkeyboard.h"
#include "wmlabel.h"
#include "wmswitcherpanel.h"
#include <wx/panel.h>
//*)
#include "wmipeditpnl.h"

class pnlNetworkSetup: public wxPanel
{
	public:

		pnlNetworkSetup(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize, long nStyle = 0, const wxString& sEmpty = wxEmptyString);
		virtual ~pnlNetworkSetup();

		//(*Declarations(pnlNetworkSetup)
		wmLabel* m_pLbl4;
		wmButton* m_pbtnStaticDHCP;
		wmLabel* m_pLbl3;
		wmButton* m_pbtnMask;
		wmipeditpnl* m_ppnlAddress;
		wmLabel* m_plblResult;
		wmKeyboard* m_pkbd;
		wmipeditpnl* m_ppnlGateway;
		wmLabel* m_pLbl1;
		wmSwitcherPanel* m_pSwp1;
		wxPanel* Panel1;
		wmButton* m_pbtnApply;
		wmLabel* m_pLbl2;
		//*)

	protected:

		//(*Identifiers(pnlNetworkSetup)
		static const long ID_M_PBTN1;
		static const long ID_M_PLBL1;
		static const long ID_M_PLBL2;
		static const long ID_M_PLBL4;
		static const long ID_PANEL3;
		static const long ID_M_PKBD1;
		static const long ID_M_PLBL3;
		static const long ID_M_PBTN3;
		static const long ID_M_PLBL5;
		static const long ID_M_PBTN2;
		static const long ID_PANEL1;
		static const long ID_M_PSWP1;
		static const long ID_PANEL2;
		//*)

	private:

		//(*Handlers(pnlNetworkSetup)
		void OntnStaticDHCPClick(wxCommandEvent& event);
		void OnedtAddressTextEnter(wxCommandEvent& event);
		void OnedtSubnetTextEnter(wxCommandEvent& event);
		void OnedtGatewayText(wxCommandEvent& event);
		void OnbtnApplyClick(wxCommandEvent& event);
		void OnbtnCancelClick(wxCommandEvent& event);
		void OnedtGatewayTextEnter(wxCommandEvent& event);
		void OnbtnMaskClick(wxCommandEvent& event);
		void OnlstSubnetSelected(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif