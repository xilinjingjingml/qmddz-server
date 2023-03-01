/*
 CopyRight(c)2006 by Banding,Shanghai, All Right Reserved.

 @Date:	        2006/11/11
 @Created:	    15:03
 @Filename: 	PlayerCreator.h
 @File path:	z:\workspace\BDLeyoyoV2\NewDDZ\Server 
 @Author:		wolfplus
 @Version:      200611111503
	
 @Description:	
*/

#pragma once

class CPlayerCreator : public IPlayerCreator
{
public:
	SINGLETON(CPlayerCreator);
	CPlayerCreator(void);
	~CPlayerCreator(void);
	virtual IPlayerDelegate* CreatePlayer();
};
