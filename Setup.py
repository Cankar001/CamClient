import os
import platform
import subprocess

if __name__ == '__main__':
    print('Your detected System is: ' + platform.system())
    
    # If the user runs the script for the first time, make sure all dependencies exist
    if (not os.path.exists('CamClient/bin')):
        print('Running the installer for the first time, making sure all lfs data is up-to-date...')
        subprocess.call(["git", "lfs", "pull"])
        subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

    print('Running premake...')

    if (platform.system() == 'Windows'):
        subprocess.call(["vendor/bin/premake/Windows/premake5.exe", "vs2022"])
    elif (platform.system() == 'Linux'):
        subprocess.call(["chmod", "+x", "vendor/bin/premake/Linux/premake5"])
        subprocess.call(["vendor/bin/premake/Linux/premake5", "gmake"])
    elif (platform.system() == 'Darwin'):
        subprocess.call(["chmod", "+x", "vendor/bin/premake/MacOS/premake5"])
        subprocess.call(["vendor/bin/premake/MacOS/premake5", "xcode4"])
        
