import subprocess, py7zr, shutil, time, sys, os
from py7zr import SevenZipFile
from pyppmd import compress

WINDEPLOYQT_PATH:str = "C:\\Qt\\6.2.3\\msvc2019_64\\bin\\windeployqt.exe"
TARGET_EXECUTABLE_NAME:str = "overlayer.exe"
BUILDS_DIRECTORY:str = "..\\builds"

BUILD_NAMES:list = os.listdir(BUILDS_DIRECTORY)

BUILD_PATHS:list = [
    os.path.abspath(os.path.join(BUILDS_DIRECTORY, build_name)) for build_name in BUILD_NAMES
]

BUILDS: dict = {(os.path.basename(build_path), build_path) for build_path in BUILD_PATHS}

BLOAT_LIST:list = [
    "opengl32sw.dll",
    "translations",
    "imageformats",
    "iconengines"
]

STYLESHEET_FILES:list = [
    "..\\submodules\\indigo-stylesheet\\indigo.qss",
    "..\\submodules\\indigo-stylesheet\\indigo.rcc"
]

def invoke_windeployqt(target_executable:str, windeployqt_path:str = None) -> int:
    global WINDEPLOYQT_PATH

    if not windeployqt_path:
        windeployqt_path = WINDEPLOYQT_PATH

    if os.path.isfile(windeployqt_path) and os.path.isfile(target_executable):
        subprocess.run([windeployqt_path, target_executable])
        return 0
    else:
        return 1

if __name__ == "__main__":
    for build_name, build_path in BUILDS:
        print("Creating release for build:", build_name)
        print("="*100)

        build_executable_path:str = "{0}/release/{1}".format(build_path, TARGET_EXECUTABLE_NAME)
        build_executable_name:str = os.path.basename(build_executable_path)

        if not os.path.isfile(build_executable_path):
            print("!WARNING! Cannot find executable for build", build_name, "at location", build_executable_path, " -- skipping this build!")
            continue

        if not os.path.isdir(build_name):
            os.mkdir(build_name)
            print("Creating directory for build @", os.path.abspath(build_name))

        release_executable_path:str = os.path.abspath("{0}/{1}".format(build_name, TARGET_EXECUTABLE_NAME))

        shutil.copy(build_executable_path, build_name)

        windeployqt_invoked:bool = False

        if input("Invoke windeployqt? [Y]/N:").lower() not in ("n", "no"):
            error_code:int = invoke_windeployqt(release_executable_path, WINDEPLOYQT_PATH)

            if error_code:
                print("!WARNING! Failed to invoke windeployqt! Code: {0}")
            else:                
                time.sleep(1)
                print("-"*100)
                if input("Remove windeployqt blaot? [Y]/N: ").lower() not in ("n", "no"):
                    for bloat_name in BLOAT_LIST:
                        bloat_path:str = os.path.join(build_name, bloat_name)

                        if os.path.isfile(bloat_path):
                            print("Removing file..", bloat_path)
                            os.remove(bloat_path)
                        elif os.path.isdir(bloat_path):
                            print("Removing directory..", bloat_path)
                            shutil.rmtree(bloat_path)
                    print("Done removing windeployqt bloat")

        print("-"*100)
        if input("Copy stylesheet file? [Y]/N: ").lower() not in ("n", "no"):
            for stylesheet_file in STYLESHEET_FILES:
                print("Copying stylesheet file..", stylesheet_file)
                shutil.copy(stylesheet_file, "{0}\\styles".format(build_name))                
            print("Done copying stylesheet files.")

        print("-"*100)
        if input("Create compressed 7z archive? [Y]/N: ").lower() not in ("n", "no"):
            archive_name:str = "{build_name}.7z".format(build_name=build_name)
            archive:SevenZipFile = SevenZipFile(archive_name, "w")

            for dir_entry in os.listdir(build_name):
                dir_entry_path:str = os.path.join(build_name, dir_entry)
                print("[{0}] Adding {1}".format(archive_name, dir_entry_path))

                if os.path.isfile(dir_entry_path):
                    archive.write(dir_entry_path, dir_entry)

                elif os.path.isdir(dir_entry_path):
                    archive.writeall(dir_entry_path, dir_entry)

            archive.close()
            print("[{0}] Finished adding files to archive.".format(archive_name))
