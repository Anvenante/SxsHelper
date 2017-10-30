#ifndef _MAIN_DLG_H_
#define _MAIN_DLG_H_

class CAssemblyNode;

class CMainDlg : public CWindowImpl<CMainDlg, CWindow, CFrameWinTraits>
{
public:
    DECLARE_WND_CLASS(TEXT("CSxsWnd"))

    BEGIN_MSG_MAP(CMainDlg)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(CFindDlg::WM_FINDMESSAGE, OnFind)
        COMMAND_ID_HANDLER(IDM_EXPORT, OnExport)
        COMMAND_ID_HANDLER(IDM_SEARCH, OnSearch)
        COMMAND_ID_HANDLER(IDM_FRESH, OnFresh)
        COMMAND_ID_HANDLER(IDM_FINDNEXT, OnFindNext)
        COMMAND_ID_HANDLER(IDM_FINDPREV, OnFindPrev)
        NOTIFY_CODE_HANDLER(TVN_ITEMCHANGED, OnItemChange)
        NOTIFY_CODE_HANDLER(NM_RCLICK, OnContext)
    ALT_MSG_MAP(1)
    ALT_MSG_MAP(2)
        MESSAGE_HANDLER(WM_CHAR, OnFilterChar)
    END_MSG_MAP()

public:
    static LPCTSTR GetWndCaption();
    /**
    * �Ի������
    */
    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnSysCommand(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    /**
    * TreeView
    */
    LRESULT OnItemChange(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
    LRESULT OnContext(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
    /**
    * �˵��¼�
    */
    LRESULT OnSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    LRESULT OnFindPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
    /**
    * ɸѡ���¼�
    */
    LRESULT OnFilterChar(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

    // ���ҶԻ�����
    inline BOOL FindMsg(LPMSG lpMsg) { return mFind.IsWindow() && mFind.IsDialogMessage(lpMsg); }

protected:
    /**
    * ɨ�����߳�
    */
    static DWORD CALLBACK ThreadScan(LPVOID lpParam);

    /**
    * �ݹ����ڵ�
    */
    void RecurveInsert(HTREEITEM hParent, CAssemblyNode *pParent);

    /**
    * ��鹤���߳�
    */
    BOOL IsWorking();

private:
    HANDLE m_hThread;
    HMENU m_hMenu;
    TCHAR m_szExport[MAX_PATH];

    CFindDlg mFind;
    CContainedWindowT<CWindow, CWinTraitsOR<TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_CHECKBOXES> > mTree;
    CContainedWindowT<CWindow, CWinTraitsOR<ES_AUTOHSCROLL, WS_EX_CLIENTEDGE> > mFilter;

    CAssemblyMap mMap;
    CComPtr<CAssemblyNode> mRoot;

public:
    CMainDlg(LPTSTR szPath);
    virtual ~CMainDlg();
};

#endif // _MAIN_DLG_H_