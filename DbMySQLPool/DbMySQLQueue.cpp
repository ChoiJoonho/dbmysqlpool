/* 
 * Copyright (C) 2012 Yee Young Han <websearch@naver.com> (http://blog.naver.com/websearch)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "SipPlatformDefine.h"
#include "DbMySQLQueue.h"
#include "Log.h"
#include "MemoryDebug.h"

CDbMySQLQueue::CDbMySQLQueue() : m_iMaxSize(0)
{
}

CDbMySQLQueue::~CDbMySQLQueue()
{
}

/**
 * @ingroup DbPool
 * @brief ť�� SQL ���ڿ��� �Է��Ѵ�.
 * @param pszSQL SQL ���ڿ�
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CDbMySQLQueue::Insert( const char * pszSQL )
{
	bool bRes = true;

	m_clsMutex.acquire();
	if( m_iMaxSize != 0 )
	{
		if( (int)m_clsList.size() >= m_iMaxSize )
		{
			CLog::Print( LOG_ERROR, "%s queue size(%d) >= max size(%d) sql(%s) is dropped", __FUNCTION__, m_clsList.size(), m_iMaxSize, pszSQL );
			bRes = false;
		}
	}

	if( bRes )
	{
		m_clsList.push_back( pszSQL );
		if( m_clsList.size() == 1 )
		{
			m_clsMutex.signal();
		}
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup DbPool
 * @brief ť���� SQL ���ڿ��� �����´�.
 * @param strSQL	SQL ���ڿ� ���� ����
 * @param bWait		SQL ���ڿ��� ������ ������ ����ϴ°�?
 * @returns SQL ���ڿ��� �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CDbMySQLQueue::Select( std::string & strSQL, bool bWait )
{
	bool bRes = false;

	m_clsMutex.acquire();
	if( bWait )
	{
		if( m_clsList.size() == 0 )
		{
			m_clsMutex.wait();
		}
	}

	if( m_clsList.size() > 0 )
	{
		strSQL = m_clsList.front();
		m_clsList.pop_front();
		bRes = true;
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup DbPool
 * @brief ť�� ����� SQL ���ڿ��� ������ �����Ѵ�.
 * @returns ť�� ����� SQL ���ڿ��� ������ �����Ѵ�.
 */
int CDbMySQLQueue::GetSize( )
{
	int iCount;

	m_clsMutex.acquire();
	iCount = (int)m_clsList.size();
	m_clsMutex.release();

	return iCount;
}

/**
 * @ingroup DbPool
 * @brief ť�� SQL ���ڿ��� ������ ������ ����ϴ� �����忡�� �̺�Ʈ�� �����Ѵ�.
 */
void CDbMySQLQueue::Signal( )
{
	m_clsMutex.broadcast();
}

/**
 * @ingroup DbPool
 * @brief ť�� ������ �� �ִ� SQL ���ڿ��� �ִ� ������ �����Ѵ�.
 * @param iMaxSize ť�� ������ �� �ִ� SQL ���ڿ��� �ִ� ����
 */
void CDbMySQLQueue::SetMaxSize( int iMaxSize )
{
	m_iMaxSize = iMaxSize;
}
