#pragma once
#include "commonDef.h"

class UniManagerTray;

template <typename T1>
class ActionEngine
{	
public:
	ActionEngine(UniManagerTray* manager)
	 : _manager(manager) {}

	void Resolve(const JSON_Object *jData)
	{
		_obj.Resolve(jData, _manager);
	}

private:
	T1 _obj;
	UniManagerTray *_manager;
};
