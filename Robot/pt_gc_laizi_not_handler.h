#pragma once
class CUserSession;
class pt_gc_laizi_not_handler
{
public:
	pt_gc_laizi_not_handler(void);
	~pt_gc_laizi_not_handler(void);

	static void handler(const pt_gc_laizi_not& noti,CUserSession* session);
};

