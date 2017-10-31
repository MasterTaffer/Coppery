#pragma once
#include <angelscript.h>
#include <scriptany/scriptany.h>
#include <vector>
#include <cassert>
#include <set>
#include <iostream>

class ASCoroutineStack;
class ASCoroutine
{
	asIScriptEngine* engine;
public:
	void addRef()
	{
		asAtomicInc(ref);
		gcFlag = false;
	}

	void release()
	{
		
		gcFlag = false;

		if (asAtomicDec(ref) <= 0)
		{
			releaseAllReferences(nullptr);
			delete this;
		}
	}

	void setGCFlag()
	{
		gcFlag = true;
	}
	bool getGCFlag()
	{
		return gcFlag;
	}
	int getRefCount()
	{
		return ref;
	}

	void enumReferences(asIScriptEngine*)
	{

		for (auto m : mailbox)
			if (m != nullptr)
				engine->GCEnumCallback(m);
		if (context != nullptr)
			engine->GCEnumCallback(context);
	}

	void releaseAllReferences(asIScriptEngine*)
	{
		for (auto& m : mailbox)
		{
			if (m != nullptr)
			{
				m->Release();
				m = nullptr;
			}
		}

		if (context)
		{
			engine->ReturnContext(context);
			context = nullptr;
		}
	}


	ASCoroutine(asIScriptContext* ctx, ASCoroutineStack* stack) : context(ctx), stack(stack)
	{
		engine = ctx->GetEngine();
	}

	asIScriptContext* context = nullptr;
	ASCoroutineStack* stack = nullptr;
	bool finished = false;

	void run();
	

	bool isFinished()
	{
		return (finished || context == nullptr);
	}


	void send(CScriptAny* any)
	{
		if (any != nullptr)
			any->AddRef();

		mailbox.push_back(any);
	}

	

	std::vector<CScriptAny*> mailbox;

private:
	bool gcFlag = false;
	int ref = 1;
};

class ASCoroutineStack
{
	asITypeInfo* coroutineType = nullptr;
	std::set<ASCoroutine*> allCoroutines;
	asIScriptContext* currentContext = nullptr;
public:
	std::vector<ASCoroutine*> stack;
	~ASCoroutineStack()
	{
		releaseResources();
	}

	void releaseResources()
	{
		if (coroutineType)
		{
			coroutineType->Release();
			coroutineType = nullptr;
		}

		for (auto* p : allCoroutines)
		{
			p->releaseAllReferences(nullptr);
			p->release();
		}
		allCoroutines.clear();
	}

	ASCoroutine* startCoroutine(asIScriptFunction *func)
	{
		auto ctx = asGetActiveContext();
		auto ase = ctx->GetEngine();
		if (func == nullptr)
		{
			ctx->SetException("Null passed in StartCoroutine");
			return nullptr;
		}
		ASCoroutine* coroutine = new ASCoroutine(ase->RequestContext(), this);
		if (coroutineType == nullptr)
		{
			coroutineType = ase->GetTypeInfoByName("Coroutine");
			coroutineType->AddRef();
		}
		coroutine->addRef();
		allCoroutines.insert(coroutine);
		

		ase->NotifyGarbageCollectorOfNewObject(coroutine, coroutineType);

		coroutine->context->Prepare(func);
		func->Release();
		return coroutine;
	}



	int runMainThread(asIScriptContext* context)
	{
		while (true)
		{
			currentContext = context;
			int r = context->Execute();
			currentContext = nullptr;

			if (r == asEXECUTION_FINISHED)
				return r;
			if (stack.size() == 0)
				return r;

			while (stack.size() > 0)
			{
				//assert(r == asEXECUTION_SUSPENDED);

				size_t prevSize = stack.size();
				auto* cr = *stack.rbegin();
				if (cr->finished)
				{
					cr->release();
					stack.resize(stack.size() - 1);
					continue;
				}

				currentContext = cr->context;
				int r2 = cr->context->Execute();
				currentContext = nullptr;

				if (r2 == asEXECUTION_FINISHED)
				{
					cr->finished = true;
					cr->releaseAllReferences(nullptr);
					if (allCoroutines.erase(cr) >= 1)
						cr->release();
				}

				if (r2 == asEXECUTION_EXCEPTION)
				{
					int ln, col;
					const char* sec;
					ln = cr->context->GetExceptionLineNumber(&col, &sec);
					std::string message = std::string("Coroutine exception: ")+cr->context->GetExceptionString();
					cr->context->GetEngine()->WriteMessage(sec, ln, col, asMSGTYPE_WARNING, message.c_str());
	
					for (auto c: stack)
					{
						c->finished = true;
						c->releaseAllReferences(nullptr);
						c->release();
						if (allCoroutines.erase(c) >= 1)
							c->release();
					}
					stack.resize(0);
					context->Abort();
					return asEXECUTION_EXCEPTION;
				}

				size_t newSize = stack.size();
				if (newSize > prevSize) //New coroutine run called
				{
					assert(r2 == asEXECUTION_SUSPENDED);

				}
				else
				{
					cr->release();
					stack.resize(stack.size() - 1);
				}
			}
		}
	}

	void yield()
	{
		auto ctx = asGetActiveContext();
		if (stack.size() == 0)
		{
			ctx->SetException("Cannot yield the main thread");
			return;
		}
		ctx->Suspend();
	}

	CScriptAny* receive()
	{
		if (stack.size() == 0)
		{
			auto ctx = asGetActiveContext();
			ctx->SetException("Cannot receive in the main thread");
			return nullptr;
		}
		auto* cr = *stack.rbegin();
		if (cr->mailbox.size() == 0)
		{
			auto ctx = asGetActiveContext();
			ctx->SetException("Coroutine mailbox empty");
			return nullptr;

		}
		CScriptAny* csa = cr->mailbox[0];
		cr->mailbox.erase(cr->mailbox.begin());
		return csa;
	}

	unsigned int getMailboxSize()
	{
		if (stack.size() == 0)
		{
			return 0;
		}
		auto* cr = *stack.rbegin();
		return cr->mailbox.size();
	}
};

void RegisterCoroutine(asIScriptEngine* ase, ASCoroutineStack* crstack);