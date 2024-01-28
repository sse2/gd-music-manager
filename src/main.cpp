#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

#if defined(_DEBUG)
#include <Windows.h>
#endif // _DEBUG

using namespace geode::prelude;

$execute{
	srand(time(nullptr));
}

namespace {
	// does the user have a directory with songs?
	bool getRedirectedSongAvailable(const gd::string& song) {
		std::string path = fmt::format("music-manager\\{}", song.c_str());
		return std::filesystem::exists(path) && std::filesystem::is_directory(path) && !std::filesystem::is_empty(path);
	}

	// pick a random song from the directory
	gd::string getRandomSongFromPath(const gd::string& song) {
		std::string path = fmt::format("music-manager\\{}", song.c_str());
		std::vector<std::string> files;

		for (const auto& entry : std::filesystem::directory_iterator(path))
			if (!std::filesystem::is_directory(entry))
				files.push_back(entry.path().string());

		// need ..\\ because the path passed to FMOD function should be relative to the Resources folder
		return gd::string{ fmt::format("..\\{}", files.at(rand() % files.size()).data()) };
	}

	// todo; get rid of these
	std::string gLastPlayedTrack;
	std::string gLastRedirectedTrack;
	std::string gLastPlayedEffect;
	std::string gLastRedirectedEffect;
}

class $modify(FMODAudioEngine) {
	// all music passes through here, some might pass more than once per play which leads to some songs fucking up the fade in (menuLoop)
	TodoReturn loadMusic(gd::string p0, float p1, float p2, float p3, bool p4, int p5, int p6) {
		// there isn't a last playing track or it isn't one we redirected
		if (gLastPlayedTrack.empty() || ( gLastPlayedTrack != std::string{ p0 } && std::string{ p0 }.find("music-manager\\") == std::string::npos) ) {
			gLastPlayedTrack = p0;
		}

		return this->FMODAudioEngine::loadMusic(p0, p1, p2, p3, p4, p5, p6);
	}

	// todo; how the fuck is the rest of the music played? why is it played in a bunch of different ways???? stupid game
	// maybe queueStartMusic?
	TodoReturn playMusic(gd::string p0, bool p1, float p2, int p3) {
		// there is a last playing track and it is the one we just redirected
		if (!gLastPlayedTrack.empty() && gLastPlayedTrack == std::string{ p0 })
			return; // not calling original here prevents fading in when going back to the title screen. maybe it causes other issues but didn't see anything

		gLastPlayedTrack = p0.data();

		// redirecting this track, if possible
		if (getRedirectedSongAvailable(p0)) {
			// pick a random file in directory - to note; the path is relative to the Resources folder here, but our current, actual, path used by std::filesystem is the GD directory
			// therefore some extra string processing is required here...
			gd::string newString = getRandomSongFromPath(p0);
			gLastRedirectedTrack = newString;

			return this->FMODAudioEngine::playMusic(newString, p1, p2, p3);
		}

		return this->FMODAudioEngine::playMusic(p0, p1, p2, p3);
	}

	/*
	// argument list is probably wrong here so it crashes
	TodoReturn playEffectAdvanced(gd::string p0, float p1, float p2, float p3, float p4, bool p5, bool p6, int p7, int p8, int p9, int p10, bool p11, int p12, bool p13, bool p14, int p15, int p16, float p17, int p18) {
#if defined(_DEBUG)
		OutputDebugStringA("[FMODAudioEngine::playEffectAdvanced] playing named effect ");
		OutputDebugStringA(p0.c_str());
		OutputDebugStringA("\n");
#endif // _DEBUG

		if (!gLastPlayedEffect.empty() && std::string{ p0.data() }.find(gLastPlayedEffect.data()) == 0)
			return this->FMODAudioEngine::playEffectAdvanced(gLastRedirectedEffect, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);

		gLastPlayedEffect = p0;

		// redirecting this track, if possible
		if (getRedirectedSongAvailable(p0)) {
			gd::string newString = getRandomSongFromPath(p0);

#if defined(_DEBUG)
			OutputDebugStringA("[DEBUG] redirecting playing effect to ");
			OutputDebugStringA(newString.c_str());
			OutputDebugStringA("\n");
#endif // _DEBUG

			gLastRedirectedEffect = newString;

			return this->FMODAudioEngine::playEffectAdvanced(newString, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
		}

		return this->FMODAudioEngine::playEffectAdvanced(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
	}
	*/
};