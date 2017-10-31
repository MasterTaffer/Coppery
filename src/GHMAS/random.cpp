#include "random.h"
#include <cassert>

ASRandomGenerator* factoryRandomGenerator()
{
	return new ASRandomGenerator();
}

void RegisterRandom(asIScriptEngine* ase)
{
	int r;
	r = ase->RegisterObjectType("RandomGenerator", 0, asOBJ_REF);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("RandomGenerator", asBEHAVE_FACTORY, "RandomGenerator@ f()", asFUNCTION(factoryRandomGenerator), asCALL_CDECL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("RandomGenerator", asBEHAVE_ADDREF, "void f()", asMETHOD(ASRandomGenerator, addRef), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectBehaviour("RandomGenerator", asBEHAVE_RELEASE, "void f()", asMETHOD(ASRandomGenerator, release), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "void opAssign(const RandomGenerator&)", asMETHODPR(ASRandomGenerator, assign, (ASRandomGenerator*), void), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "void seed(uint)", asMETHODPR(ASRandomGenerator, seed, (uint32_t), void), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "void seed(uint[]&)", asMETHODPR(ASRandomGenerator, seed, (CScriptArray*), void), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "int getI()", asMETHOD(ASRandomGenerator, getI), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "uint getU()", asMETHOD(ASRandomGenerator, getU), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "double getD()", asMETHOD(ASRandomGenerator, getD), asCALL_THISCALL);
	assert(r >= 0);

	r = ase->RegisterObjectMethod("RandomGenerator", "void seedFromTime()", asMETHOD(ASRandomGenerator, seedFromTime), asCALL_THISCALL);
	assert(r >= 0);
}

inline void ASRandomGenerator::addRef()
{
	asAtomicInc(ref);
}

inline void ASRandomGenerator::release()
{
	if (asAtomicDec(ref) <= 0)
		delete this;
}

inline ASRandomGenerator::ASRandomGenerator()
{
	seedFromTime();
}

inline void ASRandomGenerator::seedFromTime()
{
	seed(static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
}

inline uint32_t ASRandomGenerator::getU()
{
	return twister();
}

inline int32_t ASRandomGenerator::getI()
{
	return twister();
}

inline double ASRandomGenerator::getD()
{
	return ddist(twister);
}

inline void ASRandomGenerator::seed(uint32_t seed)
{
	twister.seed(seed);
}

inline void ASRandomGenerator::seed(CScriptArray * arr)
{
	if (arr->GetElementTypeId() != asTYPEID_UINT32)
	{
		auto* ctx = asGetActiveContext();
		ctx->SetException("ASRandomGenerator::Seed array element type not uint32");
		return;
	}
	std::vector<uint32_t> vec;
	vec.reserve(arr->GetSize());
	for (unsigned int i = 0; i < arr->GetSize(); i++)
	{
		vec.push_back(static_cast<uint32_t*>(arr->GetBuffer())[i]);
	}
	std::seed_seq seq(vec.begin(), vec.end());
	twister.seed(seq);
}

inline void ASRandomGenerator::assign(ASRandomGenerator * from)
{
	twister = from->twister;
}
