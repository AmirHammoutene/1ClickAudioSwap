// EndPointController.cpp : Defines the entry point for the console application.
//

#include "SoundDeviceController.h"

using namespace std;

// EndPointController.exe [NewDefaultDeviceID]
int _tmain(int argc, LPCWSTR argv[])
{
	SoundDeviceController* pSoundDeviceController = new SoundDeviceController();

	if (argc > 1)
	{
		//Reworked by Amir Hammoutene
		LPCWSTR targetDeviceName = argv[1];
		
		pSoundDeviceController->createActiveDevices();
		int deviceIndex = pSoundDeviceController->getDeviceIndexfromName(targetDeviceName);
		if (pSoundDeviceController->getDeviceCount() > deviceIndex && deviceIndex > -1)
		{
			HRESULT result = pSoundDeviceController->setDefaultSoundDevice(deviceIndex);
			pSoundDeviceController->destroySoundDevices();//dont forget to clean up!
			return result;
		}
	}


	pSoundDeviceController->destroySoundDevices();//dont forget to clean up!

	return 0;
}