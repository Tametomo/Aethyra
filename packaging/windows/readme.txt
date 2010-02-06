HOW TO BUILD THE INSTALLER

First time setup

* Install NSIS (http://nsis.sourceforge.net)
* Install UPX (http://upx.sourceforge.net/)
  (the script expects upx to be in a "upx" subdirectory right here)

How to compile Aethyra

* Install TortoiseSVN (http://tortoisesvn.tigris.org/)
* Install Code::Blocks with the MingW libraries (http://www.codeblocks.org/)
* Get the Windows compile tools through TortoiseSVN using this repository path:
svn://209.168.213.109/builds/windows/resources
  and install it to the same directory that the code is in.
* Launch Code::Blocks and load the aethyra.cbp file in the source code directory,
  then click build > build.
* Copy the DLL files from the resources\dll folder to the same
  folder where aethyra.exe built (the same directory where aethyra.cbp is located)
* (Optional) Get the latest music files, and paste them in the data\music directory.

Building the installer

* Make sure Aethyra is compiled at the right version
* Make sure the correct DLLs are in place
* Make sure the latest music files are in place
* Make sure setup.nsi mentions the right Aethyra version
* Right-click setup.nsi and select "Compile NSIS Script"
