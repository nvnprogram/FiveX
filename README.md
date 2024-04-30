# FiveX
A Phive(physics system found in latest Nintendo games) converter, targeting Splatoon 3's version of Phive (later possibly TOTK).
It can convert .bphsh files into .obj + material info json and vice versa.

# Usage tutorial
1. Install Splatoon 3 **v1.0.0** or **v1.1.0** on Ryujinx
2. Right click the game, open mods and put the **fivex** folder from the release zip into **the mods folder for the game** (make sure you do not have code mods like Flexlion installed)
3. Make sure **guest internet access** is enabled on Ryujinx.
4. Run the game. If done correctly, the Ryujinx console should say **"Waiting for commands"**.
5. Use the **fivex.py** file from the **converter folder** from release zip.

# How to use the converter file
- The easiest option is to just drag and drop the file(s) you need into the script. For example, you can take a .bphsh file and drop it into the converter and it'll generate a .obj and .json file in the same folder and vice versa, you can select a .obj file or both a .obj and a .json file, drop it into the converter and get a .bphsh.
- You can also open cmd and run **py fivex.py -h** to get the command line syntax.

# Notes
This tool will generate files useable on all versions of Splatoon 3(the version of Splatoon 3 its run on doesn't matter).
Phive files contain information about materials, such as the material id(usually used for sound/effects), material flags(like Water), and material collision disable flags(which types of objects will be ignored by the collision). You can see how it looks like in the example config given or convert a bphsh into obj + json and look at the generated json.
The name of the material is arbitrary(you can set any you want), but when converting .obj to .bphsh, it'll be matched to the material names specified in the .obj file (the ones used for **usemtl**)
The data you can modify is **mat_name**, **mat_flags** and **col_disable_flags**. You can see the list of available options in **fxconfig.json** (it was made using enum strings from Splatoon 3's exefs).

# Credit
- [exlaunch](https://github.com/shadowninja108/exlaunch) - base for the code
- [OBJLoader](https://github.com/Bly7/OBJ-Loader) - original code for parsing .obj files
- [MrMystery](https://github.com/MrMystery-Official) - help with Phive saving