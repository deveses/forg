// EAsyncSocket.cpp: implementation of the EAsyncSocket class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EAsyncSocket.h"
#include "EWnd.h"

inline LPSTR T2A(LPTSTR lp) { return (LPSTR)lp; }

namespace emfc {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EAsyncSocket::EAsyncSocket()
{
    m_hSocket=INVALID_SOCKET;
    m_hSocketWnd=NULL;
}

EAsyncSocket::~EAsyncSocket()
{
   if (m_hSocket != INVALID_SOCKET)
      Close();
}

BOOL EAsyncSocket::Create( UINT nSocketPort, int nSocketType, long IEvent, LPCTSTR lpszSocketAddress)
{

    if (Socket(nSocketType,IEvent)) {
      if (Bind(nSocketPort,lpszSocketAddress))
         return TRUE;
      int nResult = GetLastError();
      Close();
      WSASetLastError(nResult);
    }
    return FALSE;
}

BOOL EAsyncSocket::Socket(int nSocketType, long lEvent, int nProtocolType, int nAddressFormat)
{
    m_hSocket=socket(nAddressFormat,nSocketType,nProtocolType);
    if (m_hSocket!=INVALID_SOCKET) {
        //CAsyncSocket::AttachHandle(m_hSocket, this, FALSE);
        //return AsyncSelect(lEvent);
        return TRUE;
    }

    return FALSE;
}

BOOL EAsyncSocket::Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress)
{
   SOCKADDR_IN sockAddr;
   memset(&sockAddr,0,sizeof(sockAddr));

   LPSTR lpszAscii = T2A((LPTSTR)lpszSocketAddress);
   sockAddr.sin_family = AF_INET;

   if (lpszAscii == NULL)
      sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   else
   {
      DWORD lResult = inet_addr(lpszAscii);
      if (lResult == INADDR_NONE)
      {
         WSASetLastError(WSAEINVAL);
         return FALSE;
      }
      sockAddr.sin_addr.s_addr = lResult;
   }

   sockAddr.sin_port = htons((u_short)nSocketPort);

   return Bind((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

BOOL EAsyncSocket::Bind(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
    if (bind(m_hSocket,lpSockAddr,nSockAddrLen)==0)
        return TRUE;

    return FALSE;
}

void EAsyncSocket::Close()
{
   if (m_hSocket != INVALID_SOCKET)
   {
      closesocket(m_hSocket);
      //CAsyncSocket::KillSocket(m_hSocket, this);
      m_hSocket = INVALID_SOCKET;
   }
}

BOOL EAsyncSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
   if (lpszHostAddress == NULL) return FALSE;

   SOCKADDR_IN sockAddr;
   memset(&sockAddr,0,sizeof(sockAddr));

   LPSTR lpszAscii = T2A((LPTSTR)lpszHostAddress);
   sockAddr.sin_family = AF_INET;
   sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

   if (sockAddr.sin_addr.s_addr == INADDR_NONE)
   {
      LPHOSTENT lphost;
      lphost = gethostbyname(lpszAscii);
      if (lphost != NULL)
         sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
      else
      {
         WSASetLastError(WSAEINVAL);
         return FALSE;
      }
   }

   sockAddr.sin_port = htons((u_short)nHostPort);

   return Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

BOOL EAsyncSocket::Connect(const SOCKADDR *lpSockAddr, int nSockAddrLen)
{
    if (connect(m_hSocket,lpSockAddr, nSockAddrLen)==0) return TRUE;

    return FALSE;
}

BOOL EAsyncSocket::Listen(int nConnectionBacklog)
{
    return (listen(m_hSocket,nConnectionBacklog)==0);
}

BOOL EAsyncSocket::GetPeerName(LPTSTR& rPeerAddress, UINT &rPeerPort)
{
   SOCKADDR_IN sockAddr;
   memset(&sockAddr, 0, sizeof(sockAddr));

   int nSockAddrLen = sizeof(sockAddr);
   BOOL bResult = (getpeername(m_hSocket,(SOCKADDR*)&sockAddr, &nSockAddrLen)==0);
   if (bResult)
   {
      rPeerPort = ntohs(sockAddr.sin_port);
      rPeerAddress = (LPTSTR)inet_ntoa(sockAddr.sin_addr);
   }
   return bResult;

}

BOOL EAsyncSocket::GetSockName(LPTSTR& rSocketAddress, UINT &rSocketPort)
{
   SOCKADDR_IN sockAddr;
   memset(&sockAddr, 0, sizeof(sockAddr));

   int nSockAddrLen = sizeof(sockAddr);
   BOOL bResult = (getsockname(m_hSocket,(SOCKADDR*)&sockAddr, &nSockAddrLen)==0);
   if (bResult)
   {
      rSocketPort = ntohs(sockAddr.sin_port);
      rSocketAddress = (LPTSTR)inet_ntoa(sockAddr.sin_addr);
   }
   return bResult;

}

BOOL EAsyncSocket::Accept(EAsyncSocket &rConnectedSocket, SOCKADDR *lpSockAddr, int *lpSockAddrLen)
{

    SOCKET hTemp = accept(m_hSocket, lpSockAddr, lpSockAddrLen);

    if (hTemp!=INVALID_SOCKET) {
        rConnectedSocket.m_hSocket=hTemp;
    }

    return (hTemp!=INVALID_SOCKET);
}

BOOL EAsyncSocket::AsyncSelect(long lEvent)
{
   return WSAAsyncSelect(m_hSocket, m_hSocketWnd,
      WM_SOCKET_NOTIFY, lEvent) != SOCKET_ERROR;

}

int EAsyncSocket::Send(const void *lpBuf, int nBufLen, int nFlags)
{
   return send(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}

int EAsyncSocket::Receive(void *lpBuf, int nBufLen, int nFlags)
{
   return recv(m_hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}

}