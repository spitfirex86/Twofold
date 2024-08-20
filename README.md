# Twofold

Twofold is a DLL mod loader for Rayman 2, based on the functionality previously provided by Ray2Fix.

New features include:
- increased stability
- load order customization
- disabling specific mods
- easier switching between modded and unmodded state

## Setup guide

### Game versions
- Recommended: The [GOG.com version](https://www.gog.com/game/rayman_2_the_great_escape) of the game.
- Uplay/Ubisoft Connect is not officially supported, but might work - try at your own risk.
- Retail CD/DVD versions **will not work**.

### Requirements
- [Visual C++ 2022 Redistributable](https://aka.ms/vs/17/release/vc_redist.x86.exe)
- [Ray2Fix](https://github.com/spitfirex86/Ray2Fix) - optional, but recommended

### Installation
- Download the **[latest release zip](https://github.com/spitfirex86/Twofold/releases/latest)**.
- Extract to the Rayman 2 installation directory.
- Run `Ray2x.exe` once to generate all configuration files.

### Usage
**To launch the game with mods:** run `Ray2x.exe`.  
**To launch the game unmodded:** run `Rayman2.exe`.

To change the load order or disable mods: edit `LoadOrder.cfg`. New mods will be added at the end of the file automatically.
