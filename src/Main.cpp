#include "stdafx.h"
#include "TestAppDelegate.h"

#define MYAPPLICATION_NAME "Test"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	ParticleSystem::SetTexturesPath("textures/Particles");

	Core::fileSystem.SetWriteDirectory("./write_directory");
	Core::fileSystem.MountDirectory("base_p");

	Log::log.AddSink(new Log::HtmlFileLogSink("log.htm", true));

	Core::Application::APPLICATION_NAME = MYAPPLICATION_NAME;

	Core::RunApplicationWithDelegate(new TestAppDelegate());
	
	return 0;
}
