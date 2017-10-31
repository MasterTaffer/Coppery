#include <cassert>
#include <vector>
#include "thread.h"

ASThread* getASThread()
{
	asIScriptContext *ctx = asGetActiveContext();
	ASThread* thread = static_cast<ASThread*>(ctx->GetUserData(ASThread_ContextUD));
	if (thread == nullptr)
	{
		ctx->SetException("Cannot call global thread messaging in the main thread");
		return nullptr;
	}
	return thread;
}
void sendInThread(CScriptAny* any)
{
	auto* thread = getASThread();
	if (thread == nullptr)
		return;
	thread->outgoing.send(any);
	
}

CScriptAny* receiveForeverInThread()
{
	auto* thread = getASThread();
	if (thread == nullptr)
		return nullptr;
	return thread->incoming.receiveForever();
}

CScriptAny* receiveInThread(uint64_t timeout)
{
	auto* thread = getASThread();
	if (thread == nullptr)
		return nullptr;
	return thread->incoming.receive(timeout);
}



ASThread* startThread(asIScriptFunction* func)
{
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *ase = ctx->GetEngine();
	ASThread* thread = new ASThread(ase, func);
	return thread;
}




void RegisterThread(asIScriptEngine* ase)
{
	int r = 0;

	r = ase->RegisterFuncdef("void ThreadFunction()");
	assert(r >= 0);


	r = ase->RegisterObjectType("Thread", 0, asOBJ_REF | asOBJ_GC);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_ADDREF, "void f()", asMETHOD(ASThread, addRef), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_RELEASE, "void f()", asMETHOD(ASThread, release), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(ASThread, setGCFlag), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(ASThread, getGCFlag), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(ASThread, getRefCount), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(ASThread, enumReferences), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Thread", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(ASThread, releaseAllReferences), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "bool run()", asMETHOD(ASThread, run), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "bool isFinished()", asMETHOD(ASThread, isFinished), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "void suspend()", asMETHOD(ASThread, suspend), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "void send(any &in)", asMETHOD(ASThread, send), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "any@ receiveForever()", asMETHOD(ASThread, receiveForever), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "any@ receive(uint64)", asMETHOD(ASThread, receive), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Thread", "int wait(uint64)", asMETHOD(ASThread, wait), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->SetDefaultNamespace("Threads");
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("any@ Receive(uint64 timeout)", asFUNCTION(receiveInThread), asCALL_CDECL);
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("any@ ReceiveForever()", asFUNCTION(receiveForeverInThread), asCALL_CDECL);
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("void Send(any&)", asFUNCTION(sendInThread), asCALL_CDECL);
	assert(r >= 0);


	r = ase->RegisterGlobalFunction("Thread@ CreateThread(ThreadFunction @func)", asFUNCTION(startThread), asCALL_CDECL);
	assert(r >= 0);


	r = ase->SetDefaultNamespace("");
	assert(r >= 0);
}

inline void ASMailbox::send(CScriptAny * any)
{
	if (any == nullptr)
		return;
	any->AddRef();
	dbg += 1;

	{
		std::lock_guard<std::mutex> lk(mutex);
		box.push_back(any);
	}

	cv.notify_one();
}

inline CScriptAny * ASMailbox::receiveForever()
{
	CScriptAny* p = nullptr;
	do
	{
		p = receive(10000);
		if (p == nullptr)
		{
			//Debug logging here
		}
	} while (p == nullptr);
	return p;
}

inline CScriptAny * ASMailbox::receive(uint64_t timeout)
{
	std::unique_lock<std::mutex> lk(mutex);
	auto dur = std::chrono::milliseconds(timeout);
	if (cv.wait_for(lk, dur, [&] {return box.size() != 0; }))
	{
		auto* p = box.front();
		box.erase(box.begin());
		return p;
	}
	return nullptr;
}

inline void ASMailbox::enumReferences(asIScriptEngine * engine)
{
	for (auto m : box)
		if (m != nullptr)
			engine->GCEnumCallback(m);
}

inline int ASMailbox::boxSize()
{
	return box.size();
}

inline void ASMailbox::release()
{
	std::lock_guard<std::mutex> lk(mutex);
	for (auto f : box)
		if (f != nullptr)
			f->Release();
	box.clear();
}

inline void ASThread::static_internal_run(ASThread * t)
{
	t->internal_run();
}

inline void ASThread::internal_run()
{
	{
		std::lock_guard<std::mutex> lk(mutex);

		if (!func)
		{
			running = false;
			release();
			return;
		}

		if (context == nullptr)
			context = engine->RequestContext();

		context->Prepare(func);
		context->SetUserData(this, ASThread_ContextUD);
	}
	context->Execute();

	{
		std::lock_guard<std::mutex> lk(mutex);
		running = false;
		context->SetUserData(nullptr, ASThread_ContextUD);
		engine->ReturnContext(context);
		context = nullptr;
		finished_cv.notify_all();
	}
	release();
	asThreadCleanup();
}

inline void ASThread::addRef()
{
	gcFlag = false;
	asAtomicInc(ref);
}

inline void ASThread::suspend()
{
	std::lock_guard<std::mutex> lk(mutex);
	if (context)
	{
		context->Suspend();
	}
}

inline void ASThread::release()
{
	gcFlag = false;

	if (asAtomicDec(ref) <= 0)
	{
		releaseAllReferences(nullptr);

		if (thread.joinable())
			thread.join();
		delete this;
	}
}

inline void ASThread::setGCFlag()
{
	gcFlag = true;
}

inline bool ASThread::getGCFlag()
{
	return gcFlag;
}

inline int ASThread::getRefCount()
{
	return ref;
}

inline void ASThread::enumReferences(asIScriptEngine *)
{
	incoming.enumReferences(engine);
	outgoing.enumReferences(engine);
	engine->GCEnumCallback(engine);
	if (context != nullptr)
		engine->GCEnumCallback(context);
	if (func != nullptr)
		engine->GCEnumCallback(func);
}

inline int ASThread::wait(uint64_t timeout)
{
	{
		std::lock_guard<std::mutex> lk(mutex);
		if (!thread.joinable())
			return -1;
	}
	{
		std::unique_lock<std::mutex> lk(mutex);
		auto dur = std::chrono::milliseconds(timeout);
		if (finished_cv.wait_for(lk, dur, [&] {return !running; }))
		{
			thread.join();
			return 1;
		}
	}
	return 0;
}

inline void ASThread::send(CScriptAny * any)
{
	incoming.send(any);
}

inline CScriptAny * ASThread::receiveForever()
{
	return outgoing.receiveForever();
}

inline CScriptAny * ASThread::receive(uint64_t timeout)
{
	return outgoing.receive(timeout);
}

inline bool ASThread::isFinished()
{
	std::lock_guard<std::mutex> lk(mutex);
	if (running)
		return false;
	return true;
}

inline bool ASThread::run()
{
	std::lock_guard<std::mutex> lk(mutex);
	if (!func)
		return false;
	if (running)
		return false;
	running = true;
	thread = std::thread(static_internal_run, this);
	addRef();
	return true;
}

inline ASThread::ASThread(asIScriptEngine * engine, asIScriptFunction * func) : engine(engine), func(func)
{
}

inline void ASThread::releaseAllReferences(asIScriptEngine *)
{
	std::lock_guard<std::mutex> lk(mutex);
	outgoing.release();
	incoming.release();
	if (func)
		func->Release();
	if (context)
		engine->ReturnContext(context);

	engine = nullptr;
	context = nullptr;
	func = nullptr;

}
