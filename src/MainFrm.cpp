#include "SxsHelper.h"

// ע����ҶԻ����¼�
UINT CFindDlg::WfindDlgMESSAGE = ::RegisterWindowMessage(FINDMSGSTRING);

CMainFrm::CMainFrm(LPCTSTR szPath) :
m_hThread(NULL),
m_hMenu(NULL),
m_hFont(NULL),
treeView(WC_TREEVIEW, this, 1),
filterEdit(WC_EDIT, this, 2)
{
    ZeroMemory(&szExport, sizeof(szExport));
    nodeRoot.szName = szPath;
}

LPCTSTR CMainFrm::GetWndCaption()
{
    static CAtlString szTitle;
    if (szTitle.IsEmpty()) szTitle.LoadString(IDR_MAIN);
    return szTitle;
}

LRESULT CMainFrm::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    HRESULT hr = S_OK;
    CAtlString szText;

    // ����ϵͳ�˵�
    HMENU hMenu = GetSystemMenu(FALSE);
    BOOL_CHECK(szText.LoadString(IDS_ABOUT));
    BOOL_CHECK(::InsertMenu(hMenu, 0, MF_SEPARATOR, IDM_ABOUT, NULL));
    BOOL_CHECK(::InsertMenu(hMenu, 0, MF_STRING, IDM_ABOUT, szText));

    // ���ô���ͼ��
    SetIcon(::LoadIcon(_Module.m_hInst, MAKEINTRESOURCE(IDR_MAIN)));
    BOOL_CHECK(CenterWindow());

    HDC hDC = GetDC();
    int nSize = ::MulDiv(16, ::GetDeviceCaps(hDC, LOGPIXELSY), 72);
    ReleaseDC(hDC);
    m_hFont = ::CreateFont(nSize, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("MS Shell Dlg"));
    BOOL_CHECK(m_hFont);

    // ���ز˵���Դ
    m_hMenu = ::LoadMenu(_Module.m_hInst, MAKEINTRESOURCE(IDR_MAIN));
    BOOL_CHECK(m_hMenu);

    BOOL_CHECK(treeView.Create(m_hWnd, NULL));

    // ���� ɸѡ�� �ؼ�
    BOOL_CHECK(szText.LoadString(IDS_FILTERHINT));
    BOOL_CHECK(filterEdit.Create(treeView, NULL));
    filterEdit.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)(LPCTSTR)szText);
    filterEdit.SendMessage(EM_LIMITTEXT, MAX_PATH);
    filterEdit.SetFont(m_hFont);

    // �ӻ�ȡĬ�ϱ���·��
    BOOL_CHECK(::SHGetSpecialFolderPath(m_hWnd, szExport, CSIDL_MYDOCUMENTS, TRUE));
   
    // ����ɨ���߳�
    m_hThread = ::CreateThread(NULL, 0, CMainFrm::ThreadScan, this, 0, NULL);
    BOOL_CHECK(m_hThread);
exit:
    // ���� -1 ��ʾ���ڴ���ʧ��
    return SUCCEEDED(hr) ? 0 : -1;
}

void CMainFrm::OnFinalMessage(HWND /*hWnd*/)
{
    if (NULL != m_hFont) ::DeleteObject(m_hFont);
    // �ͷŲ˵���Դ
    if (NULL != m_hMenu) ::DestroyMenu(m_hMenu);
    ::PostQuitMessage(0);
}

LRESULT CMainFrm::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    treeView.ResizeClient(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    return filterEdit.SetWindowPos(HWND_TOP, GET_X_LPARAM(lParam) - 180, 0, 180, 25, 0);
}

LRESULT CMainFrm::OnSysCommand(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    switch (LOWORD(wParam))
    {
    case IDM_ABOUT:
    {
        ATL::CSimpleDialog<IDD_ABOUT> dlgAbout;
        return dlgAbout.DoModal(m_hWnd);
    }
    case SC_CLOSE:
        bHandled = IsWorking();
        break;
    }
    bHandled = FALSE;
    return S_OK;
}

/**
 * ��������߼�
 */
LRESULT CMainFrm::OnFind(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
    LPFINDREPLACE pfr = (LPFINDREPLACE)lParam;
    if (pfr->Flags & FR_DIALOGTERM)
    {
        findDlg.m_hWnd = NULL;
        pfr->Flags &= ~FR_DIALOGTERM;
        return TRUE;
    }

    TCHAR szText[MAX_PATH] = { 0 };
    TVITEM tvi = { TVIF_TEXT };
    tvi.pszText = szText;
    tvi.cchTextMax = _countof(szText);
    // ��ȡ��ǰ�Ľڵ�
    tvi.hItem = TreeView_GetSelection(treeView);
    if (NULL != pfr->lCustData) TreeView_SetItemState(treeView, (HTREEITEM)pfr->lCustData, 0, TVIS_BOLD);

    PTSTR(WINAPI *pStrStr)(PCTSTR, PCTSTR) = (pfr->Flags & FR_MATCHCASE) ? ::StrStr : ::StrStrI;

    // ��ǰ����
    if (NULL == tvi.hItem) tvi.hItem = TreeView_GetRoot(treeView);
    do
    {
        pfr->lCustData = (LPARAM)TreeView_GetChild(treeView, tvi.hItem);
        while (NULL == pfr->lCustData && NULL != tvi.hItem)
        {
            pfr->lCustData = (LPARAM)TreeView_GetNextSibling(treeView, tvi.hItem);
            if (NULL == pfr->lCustData) tvi.hItem = TreeView_GetParent(treeView, tvi.hItem);
        }
        if (NULL == pfr->lCustData) break;
        tvi.hItem = (HTREEITEM)pfr->lCustData;
        TreeView_GetItem(treeView, &tvi);
    } while (pStrStr(szText, pfr->lpstrFindWhat) == NULL);

    if (NULL != pfr->lCustData)
    {
        TreeView_SetItemState(treeView, tvi.hItem, TVIS_BOLD, TVIS_BOLD);
        return TreeView_SelectItem(treeView, tvi.hItem);
    }
    // ��ʾû���ҵ�
    CAtlString szEnd;
    szEnd.Format(IDS_FINDEND, pfr->lpstrFindWhat);
    return findDlg.MessageBox(szEnd, CMainFrm::GetWndCaption(), MB_ICONWARNING);
}

LRESULT CMainFrm::OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
    LPFINDREPLACE pfr = findDlg.GetNotifier();
    if (pfr->lpstrFindWhat[0] == TEXT('\0')) return FALSE;

    pfr->Flags |= FR_DOWN;
    return OnFind(WM_COMMAND, 0, (LPARAM)pfr, bHandled);
}

LRESULT CMainFrm::OnFindPrev(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& bHandled)
{
    LPFINDREPLACE pfr = findDlg.GetNotifier();
    if (pfr->lpstrFindWhat[0] == TEXT('\0')) return FALSE;

    pfr->Flags &= ~FR_DOWN;
    return OnFind(WM_COMMAND, 0, (LPARAM)pfr, bHandled);
}

/**
* ���ڵ㼶��ѡ��
*/
LRESULT CMainFrm::OnChanged(int /*idCtrl*/, LPNMHDR pnmh, BOOL& /*bHandled*/)
{
    NMTVITEMCHANGE *pChange = (NMTVITEMCHANGE *)pnmh;
    CAssemblyNode *pNode = reinterpret_cast<CAssemblyNode *>(pChange->lParam);
    if (NULL == pNode) return S_OK;

    BOOL bChecked = (pChange->uStateNew >> 12) == 2 ? TRUE : FALSE;
    if (bChecked == pNode->bCheck) return S_OK;
    // ���ڵ㼶��ѡ��
    pNode->bCheck = bChecked;
    for (int i = 0; i < pNode->Package.GetSize(); i++)
    {
        CAssemblyNode *pChild = pNode->Package.GetValueAt(i);
        for (int j = 0; j < pChild->Parent.GetSize(); j++)
        {
            TreeView_SetCheckState(pnmh->hwndFrom, pChild->Parent.GetValueAt(j), bChecked);
        }
    }
    return S_OK;
}

LRESULT CMainFrm::OnClick(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
    TVHITTESTINFO ht = { 0 };
    ::GetCursorPos(&ht.pt);
    treeView.ScreenToClient(&ht.pt);

    if (!TreeView_HitTest(treeView, &ht) || (ht.flags & TVHT_ONITEMLABEL) == 0) return S_OK;

    TVITEM tvi = { 0 };
    tvi.hItem = ht.hItem;
    tvi.mask = TVIF_PARAM;
    if (!TreeView_GetItem(treeView, &tvi)) return S_OK;

    TCHAR szWinSxS[MAX_PATH], szSearch[MAX_PATH] = { 0 };
    ::PathCombine(szWinSxS, nodeRoot.szName, TEXT("WinSxS\\Manifests"));
    return S_OK;
}

LRESULT CMainFrm::OnContext(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
{
    POINT pt = { 0, 0 };
    // �����Ҽ��˵�
    ::GetCursorPos(&pt);
    return ::TrackPopupMenu(::GetSubMenu(m_hMenu, 0), TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hWnd, NULL);
}

LRESULT CMainFrm::OnExport(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    CAtlString szFilter;
    TCHAR szExport[MAX_PATH] = { 0 };
    szFilter.LoadString(IDS_FILTERSAVE);
    szFilter.Replace(TEXT('|'), TEXT('\0'));
    ::PathCombine(szExport, szExport, (wID == IDM_EXPORT) ? TEXT("Remove.txt") : TEXT("Dump.txt"));
   
    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = m_hWnd;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrDefExt = TEXT("txt");
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFile = szExport;

    if (!::GetSaveFileName(&ofn)) return FALSE;

    HANDLE hFile = ::CreateFile(szExport, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        szFilter.Format(IDS_ERROR, ::GetLastError());
        return MessageBox(szFilter, CMainFrm::GetWndCaption(), MB_ICONERROR);
    }

    CHAR szName[MAX_PATH + 2];
    for (int i = 0; i < mapPackage.GetSize(); i++)
    {
        CAssemblyNode *pNode = mapPackage.GetValueAt(i);
        if (pNode->bCheck)
        {
            BOOL bCheck = TRUE;
            for (int j = 0; j < pNode->Parent.GetSize(); j++) if (pNode->Parent.GetKeyAt(j)->bCheck) bCheck = FALSE;
           // for (size_t j = 0; j < pNode->Depend.GetCount(); j++) if (pNode->Depend.GetAt(j)->bCheck) bCheck = FALSE;
            if (bCheck || wID == IDM_DUMP)
            {
                DWORD cbLen = (DWORD)sprintf_s(szName, _countof(szName), "%ws\r\n", (LPCTSTR)pNode->szName);
                ::WriteFile(hFile, szName, cbLen, &cbLen, NULL);
            }
        }
    }
   
    return ::CloseHandle(hFile);
}

LRESULT CMainFrm::OnSearch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // ��Ϊ�ղ��ܲ���
    if (mapPackage.GetSize() == 0) return FALSE;
    if (IsWorking()) return FALSE;
    if (NULL != findDlg.m_hWnd && findDlg.IsWindow()) return TRUE;
    return findDlg.Create(FR_DOWN, m_hWnd);
}

LRESULT CMainFrm::OnFresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    if (IsWorking()) return FALSE;

    // ɾ���ڵ�
    TreeView_DeleteAllItems(treeView);
    nodeRoot.Package.RemoveAll();
    mapPackage.RemoveAll();

    // ����ɨ���߳�
    m_hThread = ::CreateThread(NULL, 0, CMainFrm::ThreadScan, this, 0, NULL);
    return TRUE;
}

LRESULT CMainFrm::OnFilterChar(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
    if (VK_RETURN == wParam)
    {
        CAtlString szFind;
        filterEdit.GetWindowText(szFind);
        for (int i = 0; i < mapPackage.GetSize(); i++)
        {
            CAssemblyNode *pNode = mapPackage.GetValueAt(i);
            if (::StrStrI(pNode->szName, szFind) != NULL)
            {
                for (int j = 0; j < pNode->Parent.GetSize(); j++)
                {
                    TreeView_SetCheckState(treeView, pNode->Parent.GetValueAt(j), TRUE);
                    TreeView_EnsureVisible(treeView, pNode->Parent.GetValueAt(j));
                }
            }
            else
            {
                for (int j = 0; j < pNode->Parent.GetSize(); j++)
                {
                    TreeView_SetCheckState(treeView, pNode->Parent.GetValueAt(j), FALSE);
                    TreeView_Expand(treeView, pNode->Parent.GetValueAt(j), TVE_COLLAPSE);
                }
            }
        }
        return TRUE;
    }
    bHandled = FALSE;
    return TRUE;
}

BOOL CMainFrm::IsWorking()
{
    if (NULL != m_hThread && ::WaitForSingleObject(m_hThread, 0) == WAIT_TIMEOUT)
    {
        CAtlString szText;
        szText.LoadString(IDS_BUSY);
        return MessageBox(szText, CMainFrm::GetWndCaption(), MB_ICONWARNING);
    }
    return FALSE;
}