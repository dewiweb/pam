#include "wmlogevent.h"
#include <wx/log.h>
#include "settings.h"
#include <wx/file.h>
#include <wx/filename.h>

wmLog* wmLog::Get()
{
    static wmLog lg;
    return &lg;
}

wmLog::wmLog() : m_pHandler(0),m_pFileLog(0)
{
    m_timerSave.SetOwner(this, wxNewId());
    m_timerSave.Start(1000);
    Connect(m_timerSave.GetId(), wxEVT_TIMER, (wxObjectEventFunction)&wmLog::OnTimerSave);

}

void wmLog::SetTarget(wxEvtHandler* pHandler)
{
    m_pHandler = pHandler;
    while(m_queueEvents.empty() == false)
    {
        m_pHandler->QueueEvent(m_queueEvents.front());
        m_queueEvents.pop();
    }
}

void wmLog::Log(wxString sDevice, wxString sMessage, int nType, bool bSend)
{
    wxMutexLocker ml(m_mutex);
    wmLogEvent* plge = new wmLogEvent(sDevice, sMessage, nType, false);
    if(bSend == true)
    {
        if(m_pHandler)
        {
            m_pHandler->QueueEvent(plge);
        }
        else
        {
            m_queueEvents.push(plge);
        }
    }
    wxLogDebug(sMessage);
    m_queueFile.push(wxString::Format(wxT("%s\t%s\t%s\r\n"), wxDateTime::Now().Format(wxT("%H:%M:%S:%l")).c_str(), sDevice.Trim().c_str(), sMessage.Trim().c_str()));


}

void wmLog::OpenLogFile(bool bOpen)
{
    if(m_pFileLog)
    {
        delete m_pFileLog;
        m_pFileLog = 0;
    }
	if(bOpen)
	{
	    if(wxDirExists(Settings::Get().GetLogDirectory()) == false)
        {
            wxMkdir(Settings::Get().GetLogDirectory());
        }

		m_pFileLog = new wxFile(wxString::Format(wxT("%s/%s.log"), Settings::Get().GetLogDirectory().c_str(), wxDateTime::Now().FormatISODate().c_str()), wxFile::write_append);



		m_dtLog = wxDateTime::Today();
		m_pFileLog->Write(wxString::Format(wxT("%s\t\t--Start logging to file ---\r\n"), wxDateTime::Now().Format(wxT("%H:%M:%S:%l")).c_str()));
	}
}


void wmLog::Log(const wxString& sMessage, int nType)
{
    Log(wxEmptyString, sMessage, nType, true);
}

wmLog::~wmLog()
{
    if(m_pFileLog)
    {
        delete m_pFileLog;
    }
}

void wmLog::OnTimerSave(wxTimerEvent& event)
{
    if(m_dtLog.IsValid() == false || m_dtLog != wxDateTime::Today())
	{
		//Create new log file by closing and re-opening
		OpenLogFile(true);
	}
	while(m_queueFile.empty() == false)
    {
        m_pFileLog->Write(m_queueFile.front());
        m_queueFile.pop();
    }
}

DEFINE_EVENT_TYPE(wxEVT_WMLOG)

 wxIMPLEMENT_DYNAMIC_CLASS(wmLogEvent, wxCommandEvent);

wmLogEvent::wmLogEvent(const wmLogEvent& event) : wxCommandEvent(event)
{
    m_dtTimestamp = event.GetTimestamp();
    m_sDevice = event.GetDevice().c_str();
    m_sMessage = event.GetLogMessage().c_str();
    m_bDebug = event.IsDebug();
}

wmLogEvent::wmLogEvent(const wxString& sDevice, const wxString& sMessage, int nType, bool bDebug) : wxCommandEvent(wxEVT_WMLOG,0),
m_dtTimestamp(wxDateTime::Now()),
m_sDevice(sDevice.c_str()),
m_sMessage(sMessage.c_str()),
m_bDebug(bDebug)
{
    SetInt(nType);
}

const wxDateTime& wmLogEvent::GetTimestamp() const
{
    return m_dtTimestamp;
}

const wxString& wmLogEvent::GetDevice() const
{
    return m_sDevice;
}

const wxString& wmLogEvent::GetLogMessage() const
{
    return m_sMessage;
}

bool wmLogEvent::IsDebug() const
{
    return m_bDebug;
}

int wmLogEvent::GetLogType() const
{
    return GetInt();
}
