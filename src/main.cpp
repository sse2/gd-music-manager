#include <Geode/Geode.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

#if defined(_DEBUG)
#include <Windows.h>
#endif // _DEBUG


using namespace geode::prelude;

$execute{
	srand(time(nullptr));
}

namespace {
	// pick a random song from a directory
	std::string getRandomSongFromPath(const std::string& path) {
		std::vector<std::string> files;

		for (const auto& entry : std::filesystem::directory_iterator(path))
			if (!std::filesystem::is_directory(entry))
				files.push_back(entry.path().filename().string());

        return files.at(rand() % files.size());
	}
    
    // returns std::nullopt if we don't have a song replacement, otherwise returns a path to the redirected song
    std::optional<gd::string> getRedirectedSongPath(const char* song) {
        
        auto filename = std::filesystem::path(song).filename();
        
		std::string path = fmt::format("{}\\music-manager\\{}", geode::dirs::getGameDir().string().data(), filename);
        if (std::filesystem::exists(path)  && !std::filesystem::is_empty(path)) {
            if (std::filesystem::is_directory(path)) {
                // disgusting path formatting because the path passed to FMOD function absolutely **needs** be relative to the Resources folder
                return gd::string{ fmt::format("..\\music-manager\\{}\\{}", filename, getRandomSongFromPath(path))};
            } else {
                return gd::string{ fmt::format("..\\music-manager\\{}", filename) };
            }
        }
        return std::nullopt;
    }
    
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
        
		// redirecting this track, if possible
        auto song = getRedirectedSongPath(p0.c_str());
		if (song.has_value()) {
            gLastRedirectedTrack = p0.c_str();
            return this->FMODAudioEngine::loadMusic(song.value(), p1, p2, p3, p4, p5, p6);
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
        auto song = getRedirectedSongPath(p0.c_str());
		if (song.has_value()) {
            gLastRedirectedTrack = p0.c_str();
            return this->FMODAudioEngine::playMusic(song.value(), p1, p2, p3);
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