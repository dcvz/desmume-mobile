// wxGUI.cpp

#include "StdAfx.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
 
#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#define static const
#include "../GUI/p7zip_32.xpm"
#undef static

#undef ACTIVATE_DIALOG_TESTS

int Main1(int argc,TCHAR **argv);

#include "Windows/Registry.h"
using namespace NWindows;
using namespace NRegistry;


#include "Common/StringConvert.h"
#include "Windows/FileDir.h"
#include "Windows/Synchronization.h"

#include "ExtractRes.h"
#include "../Explorer/MyMessages.h"

#include "ExtractGUI.h"
#include "UpdateGUI.h"
#include "BenchmarkDialog.h"
#include "../FileManager/RegistryUtils.h"

using namespace NWindows;
using namespace NFile;

#include "../FileManager/ProgramLocation.h"

static LPCWSTR kHelpFileName = L"help/";

void ShowHelpWindow(HWND hwnd, LPCWSTR topicFile)
{
  UString path;
  if (!::GetProgramFolderPath(path))
    return;
  path += kHelpFileName;
  path += topicFile;
  printf("ShowHelpWindow(%p,%ls)=>%ls\n",hwnd,topicFile,(const wchar_t *)path);
  // HtmlHelp(hwnd, GetSystemString(path), HH_DISPLAY_TOPIC, NULL);
  wxString path2(path);
  wxLaunchDefaultBrowser(path2);
}

////////////////////////////// TRIES ///////////////////////////////////

#ifdef ACTIVATE_DIALOG_TESTS
static void ErrorMessage(const wchar_t *message)
{
  MessageBox(0,message, wxT("7-Zip GUI"),wxICON_ERROR);
}

#include "../FileManager/PasswordDialog.h"
#include "../FileManager/MessagesDialog.h"
#include "../FileManager/OverwriteDialog.h"
#include "Windows/Thread.h"

void myErrorMsg(const wchar_t *message)
{
	MessageBox(0,message, wxT("Message"),wxICON_ERROR);
}

void testCMessagesDialog()
{
	UStringVector Messages;

	Messages.Add(L"message 1");
	Messages.Add(L"message 2");
	Messages.Add(L"message 3");
	Messages.Add(L"message 4");
	Messages.Add(L"message 5");
	Messages.Add(L"message 6");
	Messages.Add(L"message 7");
	Messages.Add(L"message 8");
	Messages.Add(L"message 9");

	CMessagesDialog messagesDialog;
    messagesDialog.Messages = &Messages;
   int ret = messagesDialog.Create( 0  ); // ParentWindow

   	if (ret == IDOK) myErrorMsg(wxT("CMessagesDialog => IDOK"));
	else if (ret == IDCANCEL) myErrorMsg(wxT("CMessagesDialog => IDCANCEL"));
	else  myErrorMsg(wxT("CMessagesDialog => ?"));

}

void testCOverwriteDialog()
{
SYSTEMTIME systemTime;
GetSystemTime( &systemTime );


const wchar_t *existName = L"existName";
FILETIME data_existTime;
FILETIME *existTime = &data_existTime ;
UInt64 data_existSize = 1234;
UInt64 *existSize = &data_existSize;
const wchar_t *newName = L"newName";
FILETIME data_newTime;
FILETIME *newTime = &data_newTime;
UInt64 data_newSize = 45678;
UInt64 *newSize = &data_newSize;
Int32 data_answer=0;
Int32 *answer = &data_answer;

SystemTimeToFileTime( &systemTime , &data_existTime);
SystemTimeToFileTime( &systemTime , &data_newTime);

  COverwriteDialog dialog;

  dialog.OldFileInfo.Time = *existTime;
  dialog.OldFileInfo.TimeIsDefined = true; // FIXME : look again at the sample !

  dialog.OldFileInfo.SizeIsDefined = (existSize != NULL);
  if (dialog.OldFileInfo.SizeIsDefined)
    dialog.OldFileInfo.Size = *existSize;
  dialog.OldFileInfo.Name = existName;

  if (newTime == 0)
    dialog.NewFileInfo.TimeIsDefined = false;
  else
  {
    dialog.NewFileInfo.TimeIsDefined = true;
    dialog.NewFileInfo.Time = *newTime;
  }
  
  dialog.NewFileInfo.SizeIsDefined = (newSize != NULL);
  if (dialog.NewFileInfo.SizeIsDefined)
    dialog.NewFileInfo.Size = *newSize;
  dialog.NewFileInfo.Name = newName;
  
  /*
  NOverwriteDialog::NResult::EEnum writeAnswer = 
    NOverwriteDialog::Execute(oldFileInfo, newFileInfo);
  */
  INT_PTR writeAnswer = dialog.Create(NULL); // ParentWindow doesn't work with 7z
  
  switch(writeAnswer)
  {
  case IDCANCEL: myErrorMsg(wxT("COverwriteDialog => IDCANCEL")); break;
  case IDNO:     myErrorMsg(wxT("COverwriteDialog => IDNO")); break;
  case IDC_BUTTON_OVERWRITE_NO_TO_ALL: myErrorMsg(wxT("COverwriteDialog => IDC_BUTTON_OVERWRITE_NO_TO_ALL")); break;
  case IDC_BUTTON_OVERWRITE_YES_TO_ALL:myErrorMsg(wxT("COverwriteDialog => IDC_BUTTON_OVERWRITE_YES_TO_ALL")); break;
  case IDC_BUTTON_OVERWRITE_AUTO_RENAME:myErrorMsg(wxT("COverwriteDialog => IDC_BUTTON_OVERWRITE_AUTO_RENAME")); break;
  case IDYES:  myErrorMsg(wxT("COverwriteDialog => IDYES")); break;
  default:  myErrorMsg(wxT("COverwriteDialog => default")); break;
  }
}

void testCPasswordDialog()
{
    CPasswordDialog dialog;

	int ret = dialog.Create(0);
	if (ret == IDOK) {
    		UString Password = dialog.Password;
		UString msg  = wxT("CPasswordDialog => IDOK password=\"");
		msg += Password;
		msg += wxT("\"");
		myErrorMsg(msg);
	}
	else if (ret == IDCANCEL) myErrorMsg(wxT("CPasswordDialog => IDCANCEL"));
	else  myErrorMsg(wxT("CPasswordDialog => ?"));

}

struct CThreadProgressDialog
{
  CProgressDialog * ProgressDialog;
  static THREAD_FUNC_DECL MyThreadFunction(void *param)
  {
    ((CThreadProgressDialog *)param)->Result = ((CThreadProgressDialog *)param)->Process();
    return 0;
  }
  HRESULT Result;
  HRESULT Process()
  {
	Sleep(1000);
	int total = 1000;

	ProgressDialog->ProgressSynch.SetTitleFileName(L"SetTitleFileName");
	ProgressDialog->ProgressSynch.SetNumFilesTotal(100);
	ProgressDialog->ProgressSynch.SetNumFilesCur(1);
	ProgressDialog->ProgressSynch.SetProgress(total, 0);
	// ProgressDialog.ProgressSynch.SetRatioInfo(inSize, outSize);
	// ProgressDialog.ProgressSynch.SetCurrentFileName(name);

	ProgressDialog->ProgressSynch.SetPos(total/10);
	ProgressDialog->ProgressSynch.SetCurrentFileName(L"File1");
	Sleep(1000);
	ProgressDialog->ProgressSynch.SetPos(total/2);
	ProgressDialog->ProgressSynch.SetCurrentFileName(L"File2");
	Sleep(1000);
	ProgressDialog->ProgressSynch.SetPos(total);
	ProgressDialog->ProgressSynch.SetCurrentFileName(L"File3");
	Sleep(1000);
	ProgressDialog->MyClose();
	return 0;
  }
};

void testCProgressDialog()
{
  CProgressDialog ProgressDialog;

  CThreadProgressDialog benchmarker;
  benchmarker.ProgressDialog = &ProgressDialog;
  NWindows::CThread thread;
  thread.Create(CThreadProgressDialog::MyThreadFunction, &benchmarker);

  //  void StartProgressDialog(const UString &title)
  int ret = ProgressDialog.Create(L"testCProgressDialog", 0);

	if (ret == IDOK) myErrorMsg(wxT("CProgressDialog => IDOK"));
	else if (ret == IDCANCEL) myErrorMsg(wxT("CProgressDialog => IDCANCEL"));
	else  myErrorMsg(wxT("CProgressDialog => ?"));

}

void testDialog(int num)
{
	NWindows::NControl::CModalDialog dialog;

	printf("Generic Dialog(%d)\n",num);
	int ret = dialog.Create(num, 0);
	if (ret == IDOK) myErrorMsg(wxT("Generic Dialog => IDOK"));
	else if (ret == IDCANCEL) myErrorMsg(wxT("Generic Dialog => IDCANCEL"));
	else  myErrorMsg(wxT("Generic Dialog => ?"));
}

void testMessageBox()
{
	int ret = MessageBoxW(0, L"test yes/no/cancel", 
            L"7-Zip", MB_YESNOCANCEL | MB_ICONQUESTION | MB_TASKMODAL);
	if (ret == IDYES) myErrorMsg(wxT("MessageBoxW => IDYES"));
	else if (ret == IDNO) myErrorMsg(wxT("MessageBoxW => IDNO"));
	else if (ret == IDCANCEL) myErrorMsg(wxT("MessageBoxW => IDCANCEL"));
	else  myErrorMsg(wxT("MessageBoxW => ?"));
}

static void testRegistry()
{
	SaveRegLang(L"fr");

	UString langFile;
	ReadRegLang(langFile);

	printf("testRegistry : -%ls-\n",(const wchar_t *)langFile);
}


int Main2(int argc,TCHAR **argv);

int Main3(int argc,wxChar **argv)
{
	testRegistry();

	int num = -1;

	if (argc >=2 )
	{
		num = argv[1][0] - L'0';
	}
	printf("num=%d\n",num);


	switch(num)
	{
		case 0:
		{
			TCHAR **argv2 = (TCHAR **)calloc(argc,sizeof(*argv));

			argv2[0] = argv[0];
			for(int i = 2; i < argc; i++) argv2[i-1] = argv[i];

			return Main2(argc-1,argv2);
		}
	// TODO Benchmark
	// TODO CCompressDialog
	// TODO CExtractDialog ?
		case 1 : testCMessagesDialog();  break;
		case 2 : testCOverwriteDialog(); break;
	 	case 3 : testCPasswordDialog();  break;
		case 4 : testCProgressDialog();  break;
		case 5 : testMessageBox();  break;
		case 9 : 
			if (argc >= 3)
			{
				AString str = GetAnsiString(argv[2]);
				int num = atoi((const char*)str);
				testDialog(num);
			}
			else
			{
				printf("usage : 7zG 9 <windowID>\n");
			}
		      	break;
		default :
			printf("usage : 7zG number\n");

	};

	return 0;
}

#endif // ACTIVATE_DIALOG_TESTS

static const TCHAR *kCUBasePath = TEXT("Software/7-ZIP");
static const WCHAR *kLangValueName = L"Lang";

void SaveRegLang(const UString &langFile)
{
  CKey key;
  key.Create(HKEY_CURRENT_USER, kCUBasePath);
  key.SetValue(kLangValueName, langFile);
}

void ReadRegLang(UString &langFile)
{
  langFile.Empty();
  CKey key;
  if (key.Open(HKEY_CURRENT_USER, kCUBasePath, KEY_READ) == ERROR_SUCCESS)
    key.QueryValue(kLangValueName, langFile);
}


//////////////////////////////////

#define NEED_NAME_WINDOWS_TO_UNIX
#include "myPrivate.h" // global_use_utf16_conversion

void mySplitCommandLineW(int numArguments, TCHAR  **arguments,UStringVector &parts) {

  parts.Clear();
  for(int ind=0;ind < numArguments; ind++) {
      UString tmp = arguments[ind];
      // tmp.Trim(); " " is a valid filename ...
      if (!tmp.IsEmpty()) {
        parts.Add(tmp);
// DEBUG printf("ARG %d : '%ls'\n",ind,(const wchar_t *)tmp);
      }
  }
}

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

// Define a new frame type
class MyFrame: public wxFrame
{
public:
    // ctor
    MyFrame(wxFrame *frame, const wxString& title, int x, int y, int w, int h);
    // virtual ~MyFrame();

    // operations
    void WriteText(const wxString& text) { m_txtctrl->WriteText(text); }
    
protected:
    // callbacks
    void OnWorkerEvent(wxCommandEvent& event);
private:
    // just some place to put our messages in
    wxTextCtrl *m_txtctrl;
    DECLARE_EVENT_TABLE()
};

enum {
    WORKER_EVENT=100    // this one gets sent from the worker thread
};
    
BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(WORKER_EVENT, MyFrame::OnWorkerEvent)
    // EVT_IDLE(MyFrame::OnIdle)
END_EVENT_TABLE()

// My frame constructor
MyFrame::MyFrame(wxFrame *frame, const wxString& title,
                 int x, int y, int w, int h)
       : wxFrame(frame, wxID_ANY, title, wxPoint(x, y), wxSize(w, h))
{
	this->SetIcon(wxICON(p7zip_32));
    
#if wxUSE_STATUSBAR
    CreateStatusBar(2);
#endif // wxUSE_STATUSBAR

    m_txtctrl = new wxTextCtrl(this, wxID_ANY, _T(""), wxPoint(0, 0), wxSize(0, 0), wxTE_MULTILINE | wxTE_READONLY);
}

void myCreateHandle(int n);
wxWindow * g_window=0;

void MyFrame::OnWorkerEvent(wxCommandEvent& event)
{
	int n = event.GetInt();
	myCreateHandle(n);
}


// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

time_t g_T0 = 0;
class MyThread : public wxThread
{
	int _argc;
	TCHAR **_argv;
public:
	MyThread(int argc,TCHAR **argv): wxThread(),_argc(argc), _argv(argv) {}

	// thread execution starts here
	virtual void *Entry()
	{
#ifdef ACTIVATE_DIALOG_TESTS
		int ret = Main3(_argc,_argv);
#else
		int ret = Main1(_argc,_argv);
#endif
		exit(ret);
	}
};

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // don't parse the command-line options !
    // : if ( !wxApp::OnInit() ) return false;

  { // define P7ZIP_HOME_DIR
    extern void my_windows_split_path(const AString &p_path, AString &dir , AString &base);
    static char p7zip_home_dir[MAX_PATH];

    UString fullPath;
    NDirectory::MyGetFullPathName(wxApp::argv[0], fullPath);
    AString afullPath = GetAnsiString(fullPath);

    AString dir,name;

    my_windows_split_path(afullPath,dir,name);

    const char *dir2 = nameWindowToUnix((const char *)dir);
    snprintf(p7zip_home_dir,sizeof(p7zip_home_dir),"P7ZIP_HOME_DIR=%s/",dir2);
    p7zip_home_dir[sizeof(p7zip_home_dir)-1] = 0;
    putenv(p7zip_home_dir);
    // DEBUG printf("putenv(%s)\n",p7zip_home_dir);
  }
  global_use_utf16_conversion = 1; // UNICODE !

  g_T0 = time(0);
  // DEBUG printf("MAIN Thread : 0x%lx\n",wxThread::GetCurrentId());

   // Create the main frame window
    MyFrame *frame = new MyFrame((wxFrame *)NULL, _T("7-zip Main Window"), 50, 50, 450, 340);
   // Don't Show the frame !
   // frame->Show(true);

    SetTopWindow(frame);

    g_window = frame;

    MyThread *thread = new MyThread(wxApp::argc,wxApp::argv);
    thread->Create(); //  != wxTHREAD_NO_ERROR
    thread->Run();

  // success: wxApp::OnRun() will be called which will enter the main message
  // loop and the application will run. If we returned false here, the
  // application would exit immediately.
    return true;
}

DWORD WINAPI GetTickCount(VOID) {
	static wxStopWatch sw;
	return sw.Time();
}

//////////////////////////////////////////

#include "resource.h"
#include "ExtractRes.h"

static CStringTable g_stringTable[] =
{
  /* resource.rc */	  
  /***************/
	{ IDS_OPEN_TYPE_ALL_FILES, L"All Files" },
	{ IDS_METHOD_STORE, L"Store" },
	{ IDS_METHOD_NORMAL, L"Normal" },
	{ IDS_METHOD_MAXIMUM, L"Maximum" },
	{ IDS_METHOD_FAST, L"Fast" },
	{ IDS_METHOD_FASTEST, L"Fastest" },
	{ IDS_METHOD_ULTRA, L"Ultra" },
	{ IDS_COMPRESS_NON_SOLID, L"Non-solid" },
	{ IDS_COMPRESS_SOLID, L"Solid" },

	{ IDS_COMPRESS_UPDATE_MODE_ADD, L"Add and replace files" },
	{ IDS_COMPRESS_UPDATE_MODE_UPDATE, L"Update and add files" },
	{ IDS_COMPRESS_UPDATE_MODE_FRESH, L"Freshen existing files" },
	{ IDS_COMPRESS_UPDATE_MODE_SYNCHRONIZE, L"Synchronize files" },
	{ IDS_COMPRESS_SET_ARCHIVE_DIALOG_TITLE, L"Browse" },
	{ IDS_COMPRESS_INCORRECT_VOLUME_SIZE, L"Incorrect volume size" },
	{ IDS_COMPRESS_SPLIT_CONFIRM_MESSAGE, L"Specified volume size: {0} bytes.\nAre you sure you want to split archive into such volumes?" },

	{ IDS_PASSWORD_USE_ASCII, L"Use only English letters, numbers and special characters (!, #, $, ...) for password." },
	{ IDS_PASSWORD_PASSWORDS_DO_NOT_MATCH, L"Passwords do not match" },
	{ IDS_PASSWORD_IS_TOO_LONG, L"Password is too long" },

	{ IDS_PROGRESS_COMPRESSING, L"Compressing" },
	{ IDS_PROGRESS_TESTING, L"Testing" },
	{ IDS_MESSAGE_NO_ERRORS, L"There are no errors" },
	{ IDS_FILES_COLON, L"Files:" },
	{ IDS_FOLDERS_COLON, L"Folders:" },
	{ IDS_SIZE_COLON, L"Size:" },
	{ IDS_COMPRESSED_COLON, L"Compressed size:" },
	{ IDS_ARCHIVES_COLON, L"Archives:" },

  /* Extract.rc */	  
  /**************/
	{ IDS_CANNOT_CREATE_FOLDER , L"Cannot create folder '{0}'"},
	{ IDS_OPEN_IS_NOT_SUPORTED_ARCHIVE, L"File is not supported archive."},

	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_CRC , L"CRC failed in '{0}'. File is broken."},
	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_DATA_ERROR , L"Data error in '{0}'. File is broken"},
	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_UNSUPPORTED_METHOD , L"Unsupported compression method for '{0}'."},
	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_CRC_ENCRYPTED , L"CRC failed in encrypted file '{0}'. Wrong password?"},
	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_DATA_ERROR_ENCRYPTED , L"Data error in encrypted file '{0}'. Wrong password?"},

	{ IDS_EXTRACT_SET_FOLDER , L"Specify a location for extracted files."},
	{ IDS_MESSAGES_DIALOG_EXTRACT_MESSAGE_CANNOT_OPEN_FILE, L"Can not open output file '{0}'."},
	{ IDS_PROGRESS_EXTRACTING, L"Extracting" },

	{ IDS_CANT_OPEN_ARCHIVE , L"Can not open file '{0}' as archive"},
	{ IDS_CANT_OPEN_ENCRYPTED_ARCHIVE , L"Can not open encrypted archive '{0}'. Wrong password?"},

	{ 0 , 0 }
};

REGISTER_STRINGTABLE(g_stringTable)

