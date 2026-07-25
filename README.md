# Nexus Streamlink

A Guild Wars 2 Nexus addon that tracks your personal WvW killstreak, squad status, and player alive/downed/dead state. Data is written to text files for use with OBS or other streaming software.

## Features

- Tracks personal kills in WvW (player kills only, not NPCs)
  - Writes kill count to a configurable file in real-time
  - Automatically resets kill count to 0 when you die in WvW
- Tracks squad membership status
- Tracks player alive/downed/dead state (works in all game modes)
- In-game options panel (Nexus → Options → Addons → Nexus Streamlink) to view live status, reset the killstreak, and change output file paths

## Installation

1. Install [Nexus](https://raidcore.gg/Nexus) if you haven't already
2. Install [ArcDPS Integration](https://raidcore.gg/Addons?search=arcdps+integration) from Nexus addon library
3. Download `nexus_streamlink.dll` from the [Releases](../../releases) page
4. Place the DLL in your `<GW2 Install>/addons/` folder
5. Launch Guild Wars 2 with Nexus

## Configuration

Open the Nexus options window (default keybind `Ctrl+O`) and go to **Options → Addons → Nexus Streamlink**. There you can:

- See your current killstreak, squad status, player status, and whether you're in WvW
- Reset the killstreak counter
- Change the output file paths (saved automatically when a field loses focus)

Output paths are full paths (e.g. `C:\stream\killstreak.txt`), so files can be written
anywhere — not just inside the GW2 install directory.

Settings are stored in `<GW2>/addons/streamlink/settings.txt` as `key=value` lines
(`killstreak_path`, `squad_path`, `playerstatus_path`). The old single-line format
(just the killstreak path) is still read, and relative paths from old settings are
resolved against the GW2 install directory for backwards compatibility.

## Output Files

All files are located in `<GW2 Install>/addons/streamlink/` by default.

| File | Content | Description |
|------|---------|-------------|
| `killstreak.txt` | `0`, `1`, `2`, ... | Current WvW killstreak count. Resets to `0` on death. |
| `squad.txt` | `0` or `1` | `1` if you are in a squad or party, `0` if not. |
| `playerstatus.txt` | `alive`, `downed`, or `dead` | Your character's current alive state. Works in all game modes. |
## How It Works

- **Kill Detection**: Uses the `KILLINGBLOW` combat result from ArcDPS local events to detect when you personally kill an enemy player
- **Death/Downed/Alive Detection**: Monitors `CHANGEUP`, `CHANGEDOWN`, and `CHANGEDEAD` state changes from ArcDPS squad events
- **WvW Detection**: Uses MumbleLink shared memory to check map type and determine if you're in WvW
- **Squad Detection**: Uses Unofficial Extras squad update events to track squad membership

## API References

- [Nexus API Documentation](https://christopher-trent.com/api-docs/)
- [ArcDPS API](https://www.deltaconnected.com/arcdps/api/)

## License

MIT License - Use as you wish.

## Disclaimer

This addon only reads combat data. It does not modify game behavior or provide any gameplay advantage beyond displaying information already visible in the game's combat log.
