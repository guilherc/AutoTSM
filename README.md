MOD That Sky Game
Game: Sky: Children of the Light (Steam version)

Installation

Clone the repository:
git clone https://github.com/guilherc/AutoTSM.git

Open the project in Visual Studio Community 2022.

Set the build configuration to Release or Debug (x64 recommended).

Make sure the following JSON files are in the same folder as the executable:

coordinates.json

accounts.json

schedules.json

Compile and run the application. The executable will be located in the Debug folder inside the project directory.

Important files for the game

The repository contains two important files inside the AutoTSM\resources folder that need to be copied to the game folders:

Copy TSM.json from AutoTSM\resources to:
C:\Program Files (x86)\Steam\steamapps\common\Sky Children of the Light\mods\TSM Resources

Copy Stone.json from AutoTSM\resources to:
C:\Program Files (x86)\Steam\steamapps\common\Sky Children of the Light\mods\TSM Resources\Teleport Locations

Dependencies

This project requires:

Visual Studio 2022 with C++ and Windows SDK support installed.

The json.hpp header from the nlohmann/json library, which is included in the include folder of this repository. No additional installation is necessary.

Windows API libraries such as shlwapi.lib (linked automatically by the compiler).

Usage

Start the program and follow the console menu.

Set your screen resolution in the Options menu.

Add Steam accounts through the "Manage Accounts" menu.

Use the menu to start automation or configure schedules.

How to Contribute

We welcome contributions to improve this project, especially to develop a graphical interface (e.g., using Qt, ImGui, or similar).

To contribute:

Fork the repository.

Create a new branch for your feature:
git checkout -b feature/new-interface

Commit your changes with a clear message:
git commit -m "Add graphical interface with Qt"

Submit a Pull Request.

Please read the Contribution Guide for more details.

License

This project is licensed under the MIT License. See the LICENSE file for details.

Disclaimer

This software is for educational purposes only. Use responsibly and respect the terms of service of the game Sky: Children of the Light.

Contact

Developer: Scr1p7