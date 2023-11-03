#pragma once

#include <sapi.h>
#include <sphelper.h>

class Voice
{
public:
	Voice();

	ISpVoice* pVoice = NULL;
	HRESULT hr;

	void speak(LPCWSTR input);
};

