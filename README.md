# WvW Killstreak Tracker

A Guild Wars 2 addon that tracks your personal killstreak in World vs. World (WvW). The kill count is written to a file that can be read by OBS or other streaming software.

**Available in two versions:**
- **Nexus Addon** (Recommended) - Uses the Nexus addon framework with configurable settings
- **ArcDPS Plugin** - Direct ArcDPS plugin for minimal setup

## Features

- Tracks personal kills in WvW game mode
- Writes kill count to a configurable file in real-time
- Automatically resets count to 0 when you die
- Resets count when entering WvW
- **Nexus version**: Settings UI for configurable output path
- **Nexus version**: Milestone alerts at 5, 10, 25, 50, and 100 kills

## Installation

### Nexus Addon (Recommended)

1. Install [Nexus](https://raidcore.gg/Nexus) if you haven't already
2. Download `wvw_killstreak.dll` from the [Releases](../../releases) page
3. Place the DLL in your `<GW2 Install>/addons/` folder
4. Launch Guild Wars 2 with Nexus
5. Configure the output path in Nexus settings (Options > Addons > WvW Killstreak)

**Default output path:** `<GW2 Install>/addons/killstreak/killstreak.txt`

### ArcDPS Plugin (Legacy)

1. Install [ArcDPS](https://www.deltaconnected.com/arcdps/) if you haven't already
2. Download `arcdps_killstreak.dll` from the [Releases](../../releases) page
3. Copy the DLL to your Guild Wars 2 `bin64` folder (same location as `d3d11.dll`)
4. Launch Guild Wars 2

**Output path:** `<GW2 Install>/addons/arcdps/killstreak.txt`

## OBS Setup

1. Add a "Text (GDI+)" source
2. Check "Read from file"
3. Browse to the killstreak.txt file:
   - Nexus: `<GW2 Install>/addons/killstreak/killstreak.txt` (or your custom path)
   - ArcDPS: `<GW2 Install>/addons/arcdps/killstreak.txt`
4. Style as desired

## Building

### Requirements

- Windows 10/11
- Visual Studio 2022 (or 2019) with C++ Desktop Development workload
- CMake 3.15+

### Build Steps

**Build Nexus version (default):**
```batch
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_TYPE=NEXUS ..
cmake --build . --config Release
```

**Build ArcDPS version:**
```batch
mkdir build && cd build
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_TYPE=ARCDPS ..
cmake --build . --config Release
```

**Using the batch file (ArcDPS version):**
```batch
build.bat
```

### Cross-compiling from Linux (MinGW)

```bash
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake -DBUILD_TYPE=NEXUS ..
make
```

## How It Works

1. **Kill Detection**: Uses the `KILLINGBLOW` combat result to detect when you personally kill an enemy
2. **Death Detection**: Monitors `CHANGEDEAD` state changes on your character to reset the counter
3. **WvW Detection**: Checks team IDs and map IDs to determine if you're in WvW

## API References

- [Nexus API Documentation](https://christopher-trent.com/api-docs/)
- [ArcDPS API](https://www.deltaconnected.com/arcdps/api/)

## License

MIT License - Use as you wish.

## Disclaimer

This addon only reads combat data. It does not modify game behavior or provide any gameplay advantage beyond displaying information already visible in the game's combat log.
