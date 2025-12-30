# wallpaper-aerials-fix

A tiny macOS (Sonoma / Sequoia / Tahoe) utility that restarts the Aerials wallpaper extension (`WallpaperAerialsExtension`) on screen unlock (or lock).

This is useful when Aerial wallpapers get stuck/stop animating on newer macOS versions.

Keywords: macOS, wallpaper, Aerials, screensaver, dynamic wallpaper, Tahoe, Sequoia, Sonoma, WallpaperAerialsExtension.

## What it does

- Listens for the distributed notification `com.apple.screenIsUnlocked` (or `com.apple.screenIsLocked`).
- Finds `WallpaperAerialsExtension` by process name.
- Sends a signal (default: `SIGTERM`) so macOS respawns it.

## Notes / limitations

- A brief wallpaper reset/flash can happen. This is inherent to stopping the extension process.
- This is an unofficial workaround. Apple may change process names/behavior anytime.

## Build

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
