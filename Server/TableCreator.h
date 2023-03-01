/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2006/11/11
 @Created:	    15:03
 @Filename: 	TableCreator.h
 @File path:	z:\workspace\BDLeyoyoV2\NewDDZ\Server 
 @Author:		wolfplus
 @Version:      200611111503
	
 @Description:	
*/

#pragma once

class CTableCreator : public ITableCreator
{
public:
	SINGLETON(CTableCreator);
	CTableCreator(void);
	~CTableCreator(void);
	virtual ITableDelegate* CreateTable();

};
