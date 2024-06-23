#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

$execute
{
 srand ( time ( nullptr ) );
}

namespace
{
 // does the user have a directory with songs?
 bool getRedirectedSongAvailable ( const gd::string &song )
 {
  std::string path = fmt::format ( "{}\\music-manager\\{}", geode::dirs::getGameDir ( ).string ( ).data ( ), song.c_str ( ) );
  return std::filesystem::exists ( path ) && ( std::filesystem::is_directory ( path ) || std::filesystem::is_regular_file ( path ) ) && !std::filesystem::is_empty ( path );
 }

 // pick a random song from the directory
 gd::string getRandomSongFromPath ( const gd::string &song )
 {
  std::string path = fmt::format ( "{}\\music-manager\\{}", geode::dirs::getGameDir ( ).string ( ).data ( ), song.c_str ( ) );
  std::vector< std::string > files;

  // path could also be a file, so we need to check for that
  if ( !std::filesystem::is_directory ( path ) )
   return gd::string ( fmt::format ( "..\\music-manager\\{}", song.c_str ( ) ).data ( ) );

  for ( const auto &entry : std::filesystem::directory_iterator ( path ) )
   if ( !std::filesystem::is_directory ( entry ) )
    files.push_back ( entry.path ( ).filename ( ).string ( ) );

  // disgusting path formatting because the path passed to FMOD function absolutely **needs** be relative to the Resources folder
  return gd::string { fmt::format ( "..\\music-manager\\{}\\{}", song.c_str ( ), files.at ( rand ( ) % files.size ( ) ) ).data ( ) };
 }

 // todo; get rid of these
 std::string gLastPlayedTrack;
 std::string gLastRedirectedTrack;
 std::string gLastPlayedEffect;
 std::string gLastRedirectedEffect;
} // namespace

// clang-format off
class $modify ( FMODAudioEngine ) {
  // all music passes through here, some might pass more than once per play which leads to some songs fucking up the fade in (menuLoop)
  void loadMusic ( gd::string p0, float p1, float p2, float p3, bool p4, int p5, int p6 ) {
    // there isn't a last playing track or it isn't one we redirected
    if ( gLastPlayedTrack.empty ( ) || ( gLastPlayedTrack != std::string { p0 } && std::string { p0 }.find ( "music-manager\\" ) == std::string::npos ) ) {
      gLastPlayedTrack = p0;
    }
  
    return this->FMODAudioEngine::loadMusic ( p0, p1, p2, p3, p4, p5, p6 );
  }
  
  // todo; refactor and add support for sfx, should look into 3.0 bindings
  void playMusic ( gd::string p0, bool p1, float p2, int p3 )
  {
   // there is a last playing track and it is the one we just redirected
   if ( !gLastPlayedTrack.empty ( ) && gLastPlayedTrack == std::string { p0 } )
    return; // not calling original here prevents fading in when going back to the title screen. maybe it causes other issues but didn't see anything
  
   gLastPlayedTrack = p0.data ( );
  
   // redirecting this track, if possible
   if ( getRedirectedSongAvailable ( p0 ) )
   {
    // pick a random file in directory - to note; the path is relative to the Resources folder here, but our current, actual, path used by std::filesystem is the GD directory
    // therefore some extra string processing is required here...
    gd::string newString = getRandomSongFromPath ( p0 );
    gLastRedirectedTrack = newString;
  
    return this->FMODAudioEngine::playMusic ( newString, p1, p2, p3 );
   }
  
   return this->FMODAudioEngine::playMusic ( p0, p1, p2, p3 );
  }
};
// clang-format on