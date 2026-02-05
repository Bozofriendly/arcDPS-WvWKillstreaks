# Nexus Streamlink

A Guild Wars 2 addon that tracks your personal killstreak in World vs. World (WvW). The kill count is written to a file that can be read by OBS or other streaming software.

**Available in two versions:**
- **Nexus Streamlink** (Recommended) - Uses the Nexus addon framework with configurable settings
- **ArcDPS Plugin** - Direct ArcDPS plugin for minimal setup

## Features

- Tracks personal kills in WvW game mode
- Writes kill count to a configurable file in real-time
- Automatically resets count to 0 when you die
- Resets count when entering WvW
- **Nexus version**: Configurable output path via settings file
- **Nexus version**: Milestone alerts at 5, 10, 25, 50, and 100 kills

## Installation

### Nexus Streamlink (Recommended)

1. Install [Nexus](https://raidcore.gg/Nexus) if you haven't already
2. Install [ArcDPS Integration](https://raidcore.gg/Addons?search=arcdps+integration) from Nexus addon library
3. Download `nexus_streamlink.dll` from the [Releases](../../releases) page
4. Place the DLL in your `<GW2 Install>/addons/` folder
5. Launch Guild Wars 2 with Nexus
6. (Optional) Edit `<GW2>/addons/streamlink/settings.txt` to change output path

**Default output path:** `<GW2 Install>/addons/streamlink/killstreak.txt`

## OBS Setup

1. Add a "Text (GDI+)" source
2. Check "Read from file"
3. Browse to the killstreak.txt file:
   - Nexus: `<GW2 Install>/addons/streamlink/killstreak.txt` (or your custom path)
   - ArcDPS: `<GW2 Install>/addons/arcdps/killstreak.txt`
4. Style as desired

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
