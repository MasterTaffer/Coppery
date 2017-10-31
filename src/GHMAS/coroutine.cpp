#include "coroutine.h"

void RegisterCoroutine(asIScriptEngine* ase, ASCoroutineStack* crstack)
{
	int r = 0;

	r = ase->RegisterFuncdef("void CoroutineFunction()");
	assert(r >= 0);


	r = ase->RegisterObjectType("Coroutine", 0, asOBJ_REF | asOBJ_GC);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_ADDREF, "void f()", asMETHOD(ASCoroutine, addRef), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_RELEASE, "void f()", asMETHOD(ASCoroutine, release), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(ASCoroutine, setGCFlag), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(ASCoroutine, getGCFlag), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(ASCoroutine, getRefCount), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(ASCoroutine, enumReferences), asCALL_THISCALL);
	assert(r >= 0);
	r = ase->RegisterObjectBehaviour("Coroutine", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(ASCoroutine, releaseAllReferences), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Coroutine", "void run()", asMETHOD(ASCoroutine, run), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Coroutine", "bool isFinished()", asMETHOD(ASCoroutine, isFinished), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("Coroutine", "void send(any &in)", asMETHOD(ASCoroutine, send), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->SetDefaultNamespace("Coroutines");
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("Coroutine@ CreateCoroutine(CoroutineFunction @func)", asMETHOD(ASCoroutineStack, startCoroutine), asCALL_THISCALL_ASGLOBAL, crstack);
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("void Yield()", asMETHOD(ASCoroutineStack, yield), asCALL_THISCALL_ASGLOBAL, crstack);
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("any@ Receive()", asMETHOD(ASCoroutineStack, receive), asCALL_THISCALL_ASGLOBAL, crstack);
	assert(r >= 0);

	r = ase->RegisterGlobalFunction("uint GetMailboxSize()", asMETHOD(ASCoroutineStack, getMailboxSize), asCALL_THISCALL_ASGLOBAL, crstack);
	assert(r >= 0);

	r = ase->SetDefaultNamespace("");
	assert(r >= 0);


}

void ASCoroutine::run()
{

	auto ctx = asGetActiveContext();
	if (finished)
	{
		ctx->SetException("Coroutine already finished");
		return;
	}
	if (context == nullptr)
	{
		ctx->SetException("Coroutine context already released");
		return;
	}
	stack->stack.push_back(this);
	addRef();
	ctx->Suspend();
}