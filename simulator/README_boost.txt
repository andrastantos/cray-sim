Boost compilation instructions:

This is just a quick note, for full information see the Boost library 
documentation

On Windows hosts, build Boost in a Visual Studio command prompt:

===============================================================================
== Visual Studio
===============================================================================

1. Setup
========
- Extract the Boost sources to a directory
- Open up a 32-bit Visual C++ command prompt and CD into that directory
- Type 'bootstrap'

2. x86 compilation
==================
- Open up a 32-bit Visual C++ command prompt and CD into your Boost source 
  directory
- Delete the bin.v2 folder, if exists
- Type in: 
  'b2 install --libdir=c:/boost/lib --stagedir=./stage32 link=static runtime-link=static'

3. x64 compilation
==================
- Open up a 64-bit Visual C++ command prompt and CD into your Boost source 
  directory
- Delete the bin.v2 folder, if exists
- Type in: 
  'b2 install --libdir=c:/boost/lib64 --stagedir=./stage64 link=static runtime-link=static address-model=64'

4. Checking the results
=======================
If all runs above succeded, you should see the following directory structure 
on your hard drive:
 C:\BOOST
 +---include
 |   +---boost-1_60
 |       +---boost
 |           ...
 +---lib
 +---lib64
You can also check that the library sizes under lib and lib64 are different, 
indicating that they are indeed for two different targets

5. Updateing the Visual Studio property sheet
=============================================
If you're not using Boost version 1.75, update common.props:
- Change the following line:
    <BOOSTVER Condition="'$(BOOSTVER)'==''">1_75</BOOSTVER>
  according to the version you're using.
If you installed Boost into a non-default location,
you need to do additional changes:
- Search for the following line in common.props:
      <AdditionalIncludeDirectories>.;..;..\sim_lib;C:\boost\include\boost-$(BOOSTVER);..\pdcurses;$(AdditionalIncludeDirectories);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
- Replace the include directory 'c:\Boost\include\boost-$(BOOSTVER)' with the 
  appropriate path for your Boost include files.
- If you insatlled Boost into a non-default location, please make the following
  additional edits. In the same file,replace the library paths with the location 
  of your Boost installation:
  <ItemDefinitionGroup Condition="'$(Platform)'=='Win32'">
    <Link>
      <AdditionalLibraryDirectories>$(FINAL_LIBDIR);C:\Boost\lib\</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>
  <ItemDefinitionGroup Condition="'$(Platform)'=='x64'">
    <Link>
      <AdditionalLibraryDirectories>$(FINAL_LIBDIR);C:\Boost\lib64\</AdditionalLibraryDirectories>
    </Link>
  </ItemDefinitionGroup>


===============================================================================
== MinGW
===============================================================================

For MinGW the easiest route is to use the mSYS2 environment from https://www.msys2.org/.

Use 'pacman' to install boost matching your toolchain. An example would be:

  pacman -S ucrt64/mingw-w64-ucrt-x86_64-boost

or

  pacman -S mingw64/mingw-w64-x86_64-boost

===============================================================================
== Cygwin
===============================================================================

Cygwin comes with boost packages (boost-devel). Install them and you should be
good to go.
