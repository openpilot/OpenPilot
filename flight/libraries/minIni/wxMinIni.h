#ifndef WXMININI_H
#define WXMININI_H

#include <wx/wx.h>
#include "minini.h"

#if defined __linux || defined __linux__ || defined __LINUX__ \
    || defined FREEBSD || defined __FreeBSD__ || defined __OpenBSD__
  #define DIRSEP_CHAR     '/'
  #define DIRSEP_STR      "/"
#else
  #define DIRSEP_CHAR     '\\'
  #define DIRSEP_STR      "\\"
#endif

class minIni {
public:
    minIni(const wxString& name, const wxString& path=wxT(""))
        {
        if (path.Len() > 0)
            iniFilename = path;
        else
            iniFilename = wxGetCwd();
        int len = iniFilename.Len();
        if (len > 0 && iniFilename[len] != DIRSEP_CHAR)
            iniFilename += wxT(DIRSEP_STR);
        iniFilename += name;
        }

    long getl(const wxString& Section, const wxString& Key, long DefValue=0, const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        return ini_getl(Section.utf8_str(), Key.utf8_str(), DefValue, name.utf8_str());
        }

    int geti(const wxString& Section, const wxString& Key, int DefValue=0, const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        return (int)ini_getl(Section.utf8_str(), Key.utf8_str(), DefValue, name.utf8_str());
        }

    wxString gets(const wxString& Section, const wxString& Key, const wxString& DefValue=wxT(""), const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        char buffer[INI_BUFFERSIZE];
        ini_gets(Section.utf8_str(), Key.utf8_str(), DefValue.utf8_str(), buffer, INI_BUFFERSIZE, name.utf8_str());
        wxString result = wxString::FromUTF8(buffer);
        return result;
        }

    bool put(const wxString& Section, const wxString& Key, long Value, const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        return ini_putl(Section.utf8_str(), Key.utf8_str(), Value, name.utf8_str());
        }

    bool put(const wxString& Section, const wxString& Key, int Value, const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        return ini_putl(Section.utf8_str(), Key.utf8_str(), Value, name.utf8_str());
        }

    bool put(const wxString& Section, const wxString& Key, const wxString& Value, const wxString& Filename=wxT(""))
        {
        wxString name = Filename.Len() > 0 ? Filename : iniFilename;
        return ini_puts(Section.utf8_str(), Key.utf8_str(), Value.utf8_str(), name.utf8_str());
        }

private:
    wxString iniFilename;
};

#endif /* WXMININI_H */
