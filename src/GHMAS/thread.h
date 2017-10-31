#pragma once
#include <iostream>
#include <angelscript.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <scriptany/scriptany.h>

const int ASThread_EngineListUD = 551;
const int ASThread_ContextUD = 550;
class ASMailbox
{
	std::mutex mutex;
	std::condition_variable cv;
	std::vector<CScriptAny*> box;
	int dbg = 0;
public:
	void send(CScriptAny* any);
	CScriptAny* receiveForever();
	CScriptAny* receive(uint64_t timeout);

	void enumReferences(asIScriptEngine* engine);


	int boxSize();

	void release();
};

class ASThread
{
	asIScriptEngine* engine = nullptr;
	asIScriptContext* context = nullptr;
	asIScriptFunction* func = nullptr;
	std::thread thread;
	std::mutex mutex;
	bool running = false;
	std::condition_variable finished_cv;
	static void static_internal_run(ASThread* t);
	void internal_run();
	int ref = 1;
	bool gcFlag = false;
public:
	void addRef();

	void suspend();


	void release();

	void setGCFlag();
	bool getGCFlag();
	int getRefCount();

	void enumReferences(asIScriptEngine*);


	ASMailbox incoming;
	ASMailbox outgoing;

	int wait(uint64_t timeout);

	void send(CScriptAny* any);

	CScriptAny* receiveForever();

	CScriptAny* receive(uint64_t timeout);

	bool isFinished();

	bool run();

	ASThread(asIScriptEngine* engine, asIScriptFunction* func);

	void releaseAllReferences(asIScriptEngine*);
};


void RegisterThread(asIScriptEngine* ase);
