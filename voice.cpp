#include "voice.h"


Voice::Voice() {

    if (FAILED(::CoInitialize(NULL))) throw("failed to init voice");
    else {
        this->hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);

        if (SUCCEEDED(hr))
        {
            ISpObjectToken* cpToken(NULL);
            SpFindBestToken(SPCAT_VOICES, L"Gender=Female", L"", &cpToken);
            this->pVoice->SetVoice(cpToken);
            cpToken->Release();

        }
        else throw("failed to intialized voice");
    }
    this->pVoice->SetRate(3);
}

void Voice::speak(LPCWSTR input) {

    hr = pVoice->Speak(input, SPF_IS_XML, NULL);
    //pVoice->Release();
    //pVoice = NULL;

}