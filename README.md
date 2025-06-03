# AutoTSM - Automation Tool for Sky: Children of the Light (Steam mod)

**Game:** Sky: Children of the Light (Steam version)  
**Automation tool executable:** AutoTSM.exe (in the `x64\Debug` folder)

---

## For Users: How to use the tool

1. Download the executable `AutoTSM.exe` along with the JSON files that are in the same `x64\Debug` folder.

2. Also download the JSON files from the `resources` folder.

3. Keep the JSON files from the `x64\Debug` folder in the same location as the `AutoTSM.exe` executable.

4. Copy the JSON files from the `resources` folder to the game folders, as shown below:

   - Copy `TSM.json` to:  
     `C:\Program Files (x86)\Steam\steamapps\common\Sky Children of the Light\mods\TSM Resources`

   - Copy `Stone.json` to:  
     `C:\Program Files (x86)\Steam\steamapps\common\Sky Children of the Light\mods\TSM Resources\Teleport Locations`

5. To use, just run the `AutoTSM.exe` executable. No installation or additional setup required.

---

## For Developers: How to build and contribute

### Build

1. Clone the repository:  
   `git clone https://github.com/guilherc/AutoTSM.git`

2. Open the project in Visual Studio Community 2022.

3. Set build configuration to Release or Debug (x64 recommended).

4. Build the project. The `AutoTSM.exe` executable will be created in the `x64\Debug` folder.

### Dependencies

- Visual Studio 2022 with C++ and Windows SDK support.  
- `json.hpp` from [nlohmann/json](https://github.com/nlohmann/json), included in the repo.  
- Windows API libraries (linked automatically).

### How to contribute

1. Fork the repository.  
2. Create a branch for your feature:  
   `git checkout -b feature/new-interface`  
3. Commit your changes with a clear message:  
   `git commit -m "Add graphical interface with Qt"`  
4. Submit a Pull Request.

See the Contribution Guide for more details.

---

## License

This project is licensed under the MIT License. See LICENSE file for details.

---

## Disclaimer

This software is for educational purposes only. Use responsibly and respect the terms of Sky: Children of the Light.

---

## Contact

Developer: Scr1p7
