/*
 * Multimonitor APIs
 *
 * Copyright 1998 Turchanov Sergey
 */

#include "windef.h"
#include "wingdi.h"
#include "winbase.h"
#include "winuser.h"

/**********************************************************************/

#define xPRIMARY_MONITOR ((HMONITOR)0x12340042)

/***********************************************************************
 *		MonitorFromPoint
 */
HMONITOR WINAPI MonitorFromPoint(POINT ptScreenCoords, DWORD dwFlags)
{
    if ((dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST)) ||
        ((ptScreenCoords.x >= 0) &&
        (ptScreenCoords.x < GetSystemMetrics(SM_CXSCREEN)) &&
        (ptScreenCoords.y >= 0) &&
        (ptScreenCoords.y < GetSystemMetrics(SM_CYSCREEN))))
    {
        return xPRIMARY_MONITOR;
    }
    return (HMONITOR)0;
}

/***********************************************************************
 *		MonitorFromRect
 */
HMONITOR WINAPI MonitorFromRect(LPRECT lprcScreenCoords, DWORD dwFlags)
{
    if ((dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST)) ||
        ((lprcScreenCoords->right > 0) &&
        (lprcScreenCoords->bottom > 0) &&
        (lprcScreenCoords->left < GetSystemMetrics(SM_CXSCREEN)) &&
        (lprcScreenCoords->top < GetSystemMetrics(SM_CYSCREEN))))
    {
        return xPRIMARY_MONITOR;
    }
    return (HMONITOR)0;
}

/***********************************************************************
 *		MonitorFromWindow
 */
HMONITOR WINAPI MonitorFromWindow(HWND hWnd, DWORD dwFlags)
{
    WINDOWPLACEMENT wp;

    if (dwFlags & (MONITOR_DEFAULTTOPRIMARY | MONITOR_DEFAULTTONEAREST))
        return xPRIMARY_MONITOR;

    if (IsIconic(hWnd) ? 
            GetWindowPlacement(hWnd, &wp) : 
            GetWindowRect(hWnd, &wp.rcNormalPosition)) {

        return MonitorFromRect(&wp.rcNormalPosition, dwFlags);
    }

    return (HMONITOR)0;
}

/***********************************************************************
 *		GetMonitorInfoA
 */
BOOL WINAPI GetMonitorInfoA(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
{
    RECT rcWork;

    if ((hMonitor == xPRIMARY_MONITOR) &&
        lpMonitorInfo &&
        (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) &&
        SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        SetRect( &lpMonitorInfo->rcMonitor, 0, 0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN) );
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;
	
	if (lpMonitorInfo->cbSize >= sizeof(MONITORINFOEXA))
            lstrcpyA(((MONITORINFOEXA*)lpMonitorInfo)->szDevice, "DISPLAY");

        return TRUE;
    }

    return FALSE;
}

/***********************************************************************
 *		GetMonitorInfoW
 */
BOOL WINAPI GetMonitorInfoW(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
{
    RECT rcWork;

    if ((hMonitor == xPRIMARY_MONITOR) &&
        lpMonitorInfo &&
        (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) &&
        SystemParametersInfoW(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        SetRect( &lpMonitorInfo->rcMonitor, 0, 0,
                 GetSystemMetrics(SM_CXSCREEN),
                 GetSystemMetrics(SM_CYSCREEN) );
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;

        if (lpMonitorInfo->cbSize >= sizeof(MONITORINFOEXW))
            lstrcpyW(((MONITORINFOEXW*)lpMonitorInfo)->szDevice, (LPCWSTR)"D\0I\0S\0P\0L\0A\0Y\0\0");

        return TRUE;
    }

    return FALSE;
}

/***********************************************************************
 *		EnumDisplayMonitors
 */
BOOL WINAPI EnumDisplayMonitors(
        HDC             hdcOptionalForPainting,
        LPRECT         lprcEnumMonitorsThatIntersect,
        MONITORENUMPROC lpfnEnumProc,
        LPARAM          dwData)
{
    RECT rcLimit;
    SetRect( &rcLimit, 0, 0, GetSystemMetrics(SM_CXSCREEN),
             GetSystemMetrics(SM_CYSCREEN) );

    if (!lpfnEnumProc)
        return FALSE;

    if (hdcOptionalForPainting)
    {
        RECT    rcClip;
        POINT   ptOrg;

        switch (GetClipBox(hdcOptionalForPainting, &rcClip))
        {
        default:
            if (!GetDCOrgEx(hdcOptionalForPainting, &ptOrg))
                return FALSE;

            OffsetRect(&rcLimit, -ptOrg.x, -ptOrg.y);
            if (IntersectRect(&rcLimit, &rcLimit, &rcClip) &&
                (!lprcEnumMonitorsThatIntersect ||
                     IntersectRect(&rcLimit, &rcLimit, lprcEnumMonitorsThatIntersect))) {

                break;
            }
            /*fall thru */
        case NULLREGION:
             return TRUE;
        case ERROR:
             return FALSE;
        }
    } else {
        if (    lprcEnumMonitorsThatIntersect &&
                !IntersectRect(&rcLimit, &rcLimit, lprcEnumMonitorsThatIntersect)) {

            return TRUE;
        }
    }

    return lpfnEnumProc(
            xPRIMARY_MONITOR,
            hdcOptionalForPainting,
            &rcLimit,
            dwData);
}
