/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2006/11/11
 @Created:	    15:02
 @Filename: 	TableCreator.cpp
 @File path:	z:\workspace\BDLeyoyoV2\NewDDZ\Server 
 @Author:		wolfplus
 @Version:      200611111502
	
 @Description:	
*/

#include "stdafx.h"
#include "TableCreator.h"
#include "GameTable.h"

CTableCreator::CTableCreator(void)
{
}

CTableCreator::~CTableCreator(void)
{
}
ITableDelegate* CTableCreator::CreateTable()
{
	return new CGameTable();
}
