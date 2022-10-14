#pragma once
#include <QRunnable> 
#include <QQueue>
#include <QSemaphore>
#include <memory>
#include "commonDef.h"

class ProgramOperatorRunable : public QRunnable
{
public:
	ProgramOperatorRunable();
	~ProgramOperatorRunable();
	
	void run() override;

	inline void ShutDown() 
	{
		_shutDown = true;
		_semephor_ptr->release();
	};
	void AddProgramTask(const ProgramData &proData);

private:
	bool _shutDown;
	std::shared_ptr<QSemaphore> _semephor_ptr;
	QQueue<ProgramData> _queueData;
};

