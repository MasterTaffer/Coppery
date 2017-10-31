#pragma once
#include <angelscript.h>
#include <scriptarray/scriptarray.h>
#include <random>
#include <chrono>
class ASRandomGenerator
{
	std::mt19937 twister;
	std::uniform_real_distribution<double> ddist = std::uniform_real_distribution<double>(0.0, 1.0);
	int ref = 1;
public:

	void addRef();

	void release();

	ASRandomGenerator();

	void seedFromTime();

	uint32_t getU();

	int32_t getI();

	double getD();

	void seed(uint32_t seed);

	void seed(CScriptArray* arr);

	void assign(ASRandomGenerator* from);

};

void RegisterRandom(asIScriptEngine* ase);
