#include <asp/fs/fs.hpp>
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
 // pick a random song from the directory, or return std::nullopt if there's nothing to redirect to
 std::optional< std::string > getRandomSongFromPath ( const asp::fs::PathView &song )
 {
  asp::fs::Path path = geode::dirs::getGameDir ( ) / "music-manager" / song;

  if ( !asp::fs::exists ( path ) )
   return std::nullopt;

  auto metaResult = asp::fs::metadata( path );
  if ( !metaResult.isOk ( ) )
   return std::nullopt;

  asp::fs::MetadataBase meta = metaResult.unwrap ( );

  // path could also be a file, so we need to check for that
  if ( meta.isFile ( ) )
   return path.string ( );

  auto iterDirResult = asp::fs::iterDir ( path );

  if ( !iterDirResult.isOk ( ) )
   return std::nullopt;

  std::vector< asp::fs::Path > files;

  for ( const auto &entry : iterDirResult.unwrap ( ) )
   if ( entry.isOk ( ) && entry.unwrap ( ).isFile ( ) )
    files.push_back ( entry.unwrap ( ).path );


  if ( files.size ( ) == 0 ) return std::nullopt;

  return files.at ( rand ( ) % files.size ( ) ).string ( );
 }

 std::string gLastPlayedTrack;
} // namespace

// clang-format off
class $modify ( FMODAudioEngine ) {
  // todo; add support for sfx
  void loadMusic ( std::string p0, float p1, float p2, float p3, bool p4, int p5, int p6, bool p7 ) {
    if ( std::string { p0 }.find ( "music-manager\\" ) == std::string::npos ) {
      gLastPlayedTrack = p0;

      // custom songs pass the full absolute path to the file, we only care about the filename
      asp::fs::Path songPath = asp::fs::Path( p0 );
      std::optional< asp::fs::PathView > song = songPath.filename ( );

      // redirecting this track, if possible
      if ( song.has_value ( ) ) {
        // pick a random file in directory - to note; the path is relative to the Resources folder here, but our current, actual, path used by asp::fs is the GD directory
        // therefore some extra string processing is required here...
        std::optional< std::string > newString = getRandomSongFromPath ( *song );
        if ( newString.has_value ( ) ) {
          return this->FMODAudioEngine::loadMusic ( *newString, p1, p2, p3, p4, p5, p6, p7 );
        }
      }
    }

    return this->FMODAudioEngine::loadMusic ( p0, p1, p2, p3, p4, p5, p6, p7 );
  }

  void playMusic ( std::string p0, bool p1, float p2, int p3 )
  {
    // there is a last playing track and it is the one we just redirected
    if ( !gLastPlayedTrack.empty ( ) && gLastPlayedTrack == std::string { p0 } )
      return; // not calling original here prevents fading in when going back to the title screen. maybe it causes other issues but didn't see anything

    return this->FMODAudioEngine::playMusic ( p0, p1, p2, p3 );
  }
};
// clang-format on
