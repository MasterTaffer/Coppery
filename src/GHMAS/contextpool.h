#pragma once
#include <angelscript.h>
#include <vector>
#include <mutex>
class ASContextPool
{
	std::mutex mutex;
	asIScriptEngine* engine = nullptr;
	std::vector<asIScriptContext*> pool;
	void lockless_updatePool();
	void lockless_disconnect();
	bool hasExceptionCb = false;
	asSFuncPtr exceptionCb;
	void* exceptionCbObject;
	int exceptionCbCallConv;
public:
	void connect(asIScriptEngine* ase);
	void disconnect();
	void updatePool();
	void setExceptionCallback(asSFuncPtr, void*, int);
	void clearExceptionCallback();
	void returnContext(asIScriptEngine* ase, asIScriptContext* ctx);
	asIScriptContext* requestContext(asIScriptEngine* ase);
};