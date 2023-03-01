#pragma once
class CUserSession;
class pt_gc_common_not_handler
{
public:
	pt_gc_common_not_handler(void);
	~pt_gc_common_not_handler(void);

	static void handler(pt_gc_common_not& noti,CUserSession* session);
};

