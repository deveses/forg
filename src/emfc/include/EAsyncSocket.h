// EAsyncSocket.h: interface for the EAsyncSocket class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EASYNCSOCKET_H__BF187670_8CB9_4105_9460_4EB418861CB7__INCLUDED_)
#define AFX_EASYNCSOCKET_H__BF187670_8CB9_4105_9460_4EB418861CB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <WinSock.h>

namespace emfc {

class EMFC_API EAsyncSocket  
{
public:
	int Receive(void* lpBuf, int nBufLen, int nFlags=0);
	int Send(const void* lpBuf, int nBufLen, int nFlags=0);
	BOOL AsyncSelect(long lEvent);
	BOOL Accept(EAsyncSocket& rConnectedSocket,SOCKADDR* lpSockAddr, int* lpSockAddrLen);
	BOOL GetSockName(LPTSTR& rSocketAddress, UINT& rSocketPort);
	BOOL GetPeerName(LPTSTR& rPeerAddress, UINT& rPeerPort);
	BOOL Listen( int nConnectionBacklog = 5 );
	HWND m_hSocketWnd;
	BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	BOOL Connect(LPCTSTR lpszHostAddress, UINT nHostPort);
	virtual void Close();
	BOOL Bind (const SOCKADDR* lpSockAddr, int nSockAddrLen);
	BOOL Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress = NULL);
	BOOL Socket(int nSocketType=SOCK_STREAM, long lEvent =FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE,int nProtocolType = 0, int nAddressFormat = PF_INET);
	SOCKET m_hSocket;
	BOOL Create( UINT nSocketPort=0, int nSocketType=SOCK_STREAM, long IEvent=FD_READ|FD_WRITE|FD_OOB|FD_ACCEPT|FD_CONNECT|FD_CLOSE,LPCTSTR lpszSocketAddress=NULL);
	EAsyncSocket();
	virtual ~EAsyncSocket();

};

}

#endif // !defined(AFX_EASYNCSOCKET_H__BF187670_8CB9_4105_9460_4EB418861CB7__INCLUDED_)
