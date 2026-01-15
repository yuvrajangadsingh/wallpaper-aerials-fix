# wallpaper-aerials-fix

A tiny macOS (Sonoma / Sequoia / Tahoe) utility that restarts the Aerials wallpaper extension (`WallpaperAerialsExtension`) on screen unlock (or lock).

This is useful when Aerial wallpapers get stuck/stop animating on newer macOS versions.

If you are using a static wallpaper (no Live Wallpaper / Aerials), this tool will likely do nothing.


Keywords: macOS, wallpaper, Aerials, screensaver, dynamic wallpaper, Tahoe, Sequoia, Sonoma, WallpaperAerialsExtension.

## What it does

- Listens for the distributed notification `com.apple.screenIsUnlocked` (or `com.apple.screenIsLocked`).
- Finds `WallpaperAerialsExtension` by process name.
- Sends a signal (default: `SIGTERM`) so macOS respawns it.

## Notes / limitations

- A brief wallpaper reset/flash can happen. This is inherent to stopping the extension process.
- This is an unofficial workaround. Apple may change process names/behavior anytime.

## Install (recommended)

Download the latest universal macOS binary from GitHub Releases:
https://github.com/yuvrajangadsingh/wallpaper-aerials-fix/releases

```bash
curl -L -o wallpaper-aerials-fix-macos-universal.zip \
  https://github.com/yuvrajangadsingh/wallpaper-aerials-fix/releases/latest/download/wallpaper-aerials-fix-macos-universal.zip
unzip wallpaper-aerials-fix-macos-universal.zip
chmod +x wallpaper-aerials-fix
# If macOS blocks it, remove quarantine attribute:
xattr -dr com.apple.quarantine wallpaper-aerials-fix 2>/dev/null || true
./wallpaper-aerials-fix --verbose
```

## Build

### Option 1: Build with clang++ (no CMake)

```bash
mkdir -p build
clang++ -O2 -std=c++17 -arch arm64 -arch x86_64 src/main.cpp \
  -framework CoreFoundation -framework CoreServices -framework ApplicationServices \
  -o build/wallpaper-aerials-fix
```

### Option 2: Build with CMake

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
```

Binary will be at `build/wallpaper-aerials-fix`.

## Run

Default (unlock event, terminate `WallpaperAerialsExtension`):

```bash
./build/wallpaper-aerials-fix
```

Verbose:

```bash
./build/wallpaper-aerials-fix --verbose
```

Trigger on lock instead:

```bash
./build/wallpaper-aerials-fix --event lock
```

Force kill if it doesn't exit (use with care):

```bash
./build/wallpaper-aerials-fix --force-after-ms 500 --force-signal KILL
```

### Multi-monitor setup (recommended for external displays)

If you have external monitors that show black wallpaper after waking from sleep, use the `--wait-for-displays` flag. This waits for external displays to report as "ready" before killing the extension:

```bash
./build/wallpaper-aerials-fix --wait-for-displays --verbose
```

With custom timeout (default is 5000ms):

```bash
./build/wallpaper-aerials-fix --wait-for-displays --display-timeout-ms 3000 --verbose
```

## Run at login (LaunchAgent)

1) Build the binary.
2) Copy the example plist and edit the path to the binary.

```bash
cp launchd/com.wallpaper-aerials-fix.plist ~/Library/LaunchAgents/
# edit ~/Library/LaunchAgents/com.wallpaper-aerials-fix.plist
launchctl unload ~/Library/LaunchAgents/com.wallpaper-aerials-fix.plist 2>/dev/null || true
launchctl load ~/Library/LaunchAgents/com.wallpaper-aerials-fix.plist
```

To stop:

```bash
launchctl unload ~/Library/LaunchAgents/com.wallpaper-aerials-fix.plist
```

## Credits

This project is inspired by Proton0’s `WallpaperVideoExtensionFix`:
https://github.com/Proton0/WallpaperVideoExtensionFix

I started from the idea in that repo and adapted it for newer macOS wallpaper extensions (Aerials) and my own workflow.
