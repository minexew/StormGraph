
// General options
#define app_name "ToL_Launcher"
#define window_title "Tales of Lanthaia"
#define exe_name "bin\\ToLClient.exe"
#define font_name "Calibri"
#define font_colour RGB( 0, 0, 0 )

// -1 for infinite
#define wait_timeout 0

// Image & window size
#define image_width 480
#define image_height 280

// Launcher URL (for display only)
#define launcher_url "http://client.lanthaia.net/"

// Where to get the version file
#define package_name "tolcl"
#define version_file_uri "http://update.lanthaia.net/tolcl2/version"

// Local version file copy
// base = "%AppData%/." + package_name
#define versionFileName ( base + "/.version" )

// Buffer for file I/O
#define buffer_size 65536

// SPECIAL OPTIONS
#define install_openal_redist
